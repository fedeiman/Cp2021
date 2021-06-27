/* Tiny Monte Carlo by Scott Prahl (http://omlc.ogi.edu)"
 * 1 W Point Source Heating in Infinite Isotropic Scattering Medium
 * http://omlc.ogi.edu/software/mc/tiny_mc.c
 *
 * Adaptado para CP2014, Nicolas Wolovick
 */

#define _XOPEN_SOURCE 500  // M_PI

#include "params.h"
#include "wtime.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <cuda_runtime.h>
#include <curand_kernel.h>

#define CUDA_CALL(x) do { if((x) != cudaSuccess) { \
    printf("Error at %s:%d\n",__FILE__,__LINE__); \
    cudaDeviceReset(); \
    return EXIT_FAILURE;}} while(0)

char t1[] = "Tiny Monte Carlo by Scott Prahl (http://omlc.ogi.edu)";
char t2[] = "1 W Point Source Heating in Infinite Isotropic Scattering Medium";
char t3[] = "CPU version, adapted for PEAGPGPU by Gustavo Castellano"
            " and Nicolas Wolovick";

typedef struct PhotonHeat {
    float *heat;
    float *heat2;
} PhotonHeat;

__global__ void setup_kernel(curandState *state, uint64_t seed)
{
    int gtid = blockDim.x * blockIdx.x + threadIdx.x;
    curand_init(seed, gtid, 0, &state[gtid]);
}
/***
    Each Photon in a thread with its own RNG
 ***/
static __global__ void photon(PhotonHeat result, curandState *rngs)
{
    int gtid = blockDim.x * blockIdx.x + threadIdx.x;
    
    // Most of the time we'll have more threads than PHOTONS due to ceil()
    // In block calculations, only do required photons
    if(gtid <= PHOTONS){
        const float albedo = MU_S / (MU_S + MU_A);
        const float shells_per_mfp = 1e4 / MICRONS_PER_SHELL / (MU_A + MU_S);

        /* launch */
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float u = 0.0f;
        float v = 0.0f;
        float w = 1.0f;
        float weight = 1.0f;
        int prevShell = -1;
        float cachedW= 0;
        float cachedWS= 0;

        for (;;) {
            float t = -logf(curand_uniform(&rngs[gtid])); /* move */
            x += t * u;
            y += t * v;
            z += t * w;

            int shell = sqrtf(x * x + y * y + z * z) * shells_per_mfp; /* absorb */
            if (shell > SHELLS - 1) {
                shell = SHELLS - 1;
            }
            if(prevShell == shell){
                cachedW += (1.0f - albedo) * weight;
                cachedWS += (1.0f - albedo) * (1.0f - albedo) * weight * weight;
            }
            else{
              //drop cached value in global heat array 
              if(prevShell >= 0){
                atomicAdd(&result.heat[prevShell], cachedW);
                atomicAdd(&result.heat2[prevShell], cachedWS); 
              }
              cachedW = (1.0f - albedo) * weight;
              cachedWS = (1.0f - albedo) * (1.0f - albedo) * weight * weight;
              prevShell = shell;
            }

            weight *= albedo;
            if (weight < 0.001f) { /* roulette */
                //If we break we must put the cached value in the array
                if (curand_uniform(&rngs[gtid]) > 0.1f){
                    atomicAdd(&result.heat[prevShell], cachedW);
                    atomicAdd(&result.heat2[prevShell], cachedWS); 
                    break;
                }
                    
                weight *= 10;
            }
            /* New direction, rejection method */
            float xi1, xi2;
            do {
                xi1 = 2.0f * curand_uniform(&rngs[gtid]) - 1.0f;
                xi2 = 2.0f * curand_uniform(&rngs[gtid]) - 1.0f;
                t = xi1 * xi1 + xi2 * xi2;
            } while (1.0f < t);
            u = 2.0f * t - 1.0f;

            float tmp = sqrtf((1.0f - u * u) / t);
            v = xi1 * tmp;
            w = xi2 * tmp;
        }
    }
}


/***
 * Main matter
 ***/

int main(void)
{

    // heading
    /* printf("# %s\n# %s\n# %s\n", t1, t2, t3);
    printf("# Scattering = %8.3f/cm\n", MU_S);
    printf("# Absorption = %8.3f/cm\n", MU_A);
    printf("# Photons    = %8d\n#\n", PHOTONS); */
    float blocks = ceil(PHOTONS / 128);
    int num_threads = blocks * 128;

    curandState *rngs;
    CUDA_CALL(cudaMallocManaged(&rngs, num_threads * sizeof(curandState)));

    PhotonHeat global;
    CUDA_CALL(cudaMallocManaged(&global.heat, SHELLS * sizeof(float)));
    CUDA_CALL(cudaMallocManaged(&global.heat2, SHELLS * sizeof(float)));
    CUDA_CALL(cudaMemset (global.heat,0, SHELLS));
    CUDA_CALL(cudaMemset (global.heat2,0, SHELLS));

    //Initialize rngs, same seed, different sequence
    setup_kernel << < blocks, 128 >> > (rngs, time(NULL));

    cudaEvent_t start, stop;
    // start timer
    CUDA_CALL(cudaEventCreate(&start));
    CUDA_CALL(cudaEventRecord(start,0));
    
    // simulation
    photon<<<blocks, 128>>>(global, rngs);

    // stop timer
    CUDA_CALL(cudaEventCreate(&stop));
    CUDA_CALL(cudaEventRecord(stop,0));
    CUDA_CALL(cudaEventSynchronize(stop));
    float elapsed;
    CUDA_CALL(cudaEventElapsedTime(&elapsed, start,stop));
    elapsed /= 1000;

    // printf("# %lf seconds\n", elapsed);
    //printf("# %lf K photons per second\n", 1e-3 * PHOTONS / elapsed);
    printf("%lf\n", 1e-3 * PHOTONS / elapsed);
     /* printf("# Radius\tHeat\n");
    printf("# [microns]\t[W/cm^3]\tError\n");
    float t = 4.0f * M_PI * powf(MICRONS_PER_SHELL, 3.0f) * PHOTONS / 1e12;
    for (unsigned int i = 0; i < SHELLS - 1; ++i) {
        printf("%6.0f\t%12.5f\t%12.5f\n", i * (float)MICRONS_PER_SHELL,
               global.heat[i] / t / (i * i + i + 1.0 / 3.0),
               sqrt(global.heat2[i] - global.heat[i] * global.heat[i] / PHOTONS) / t / (i * i + i + 1.0f / 3.0f));
    }
    printf("# extra\t%12.5f\n", global.heat[SHELLS - 1] / PHOTONS);  */

    CUDA_CALL(cudaFree(global.heat));
    CUDA_CALL(cudaFree(global.heat2));
    CUDA_CALL(cudaFree(rngs));
    return 0;
}
