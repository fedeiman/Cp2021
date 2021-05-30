/* Tiny Monte Carlo by Scott Prahl (http://omlc.ogi.edu)"
 * 1 W Point Source Heating in Infinite Isotropic Scattering Medium
 * http://omlc.ogi.edu/software/mc/tiny_mc.c
 *
 * Adaptado para CP2014, Nicolas Wolovick
 */

#define _XOPEN_SOURCE 500  // M_PI

#include "params.h"
#include "wtime.h"
#include "pcg_basic.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

char t1[] = "Tiny Monte Carlo by Scott Prahl (http://omlc.ogi.edu)";
char t2[] = "1 W Point Source Heating in Infinite Isotropic Scattering Medium";
char t3[] = "CPU version, adapted for PEAGPGPU by Gustavo Castellano"
            " and Nicolas Wolovick";

// global state, heat and heat square in each shell
static float heat[SHELLS];
static float heat2[SHELLS];

double myrand(pcg32_random_t *rngptr){
    return ldexp(pcg32_random_r(rngptr) ,-32);
}

/***
 * Photon
 ***/

static void photon(pcg32_random_t rng_ptr)
{
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

    for (;;) {
        float t = -logf(myrand(&rng_ptr)); /* move */
        x += t * u;
        y += t * v;
        z += t * w;

        unsigned int shell = sqrtf(x * x + y * y + z * z) * shells_per_mfp; /* absorb */
        if (shell > SHELLS - 1) {
            shell = SHELLS - 1;
        }

            heat[shell] += (1.0f - albedo) * weight;
            heat2[shell] += (1.0f - albedo) * (1.0f - albedo) * weight * weight; /* add up squares */   
        weight *= albedo;
        if (weight < 0.001f) { /* roulette */
            if (myrand(&rng_ptr) > 0.1f)
                break;
            weight /= 0.1f;
        }
        /* New direction, rejection method */
        float xi1, xi2;
        do {
            xi1 = 2.0f * myrand(&rng_ptr) - 1.0f;
            xi2 = 2.0f * myrand(&rng_ptr) - 1.0f;
            t = xi1 * xi1 + xi2 * xi2;
        } while (1.0f < t);
        u = 2.0f * t - 1.0f;

        float tmp = sqrtf((1.0f - u * u) / t);
        v = xi1 * tmp;
        w = xi2 * tmp;


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

    // configure RNG
    //pcg32_random_t rng1, rng2, rng3;
    //pcg32_srandom_r(&rng1, time(NULL), (intptr_t)&rng1);
    //pcg32_srandom_r(&rng2, time(NULL), (intptr_t)&rng2);
    //pcg32_srandom_r(&rng3, time(NULL), (intptr_t)&rng3);

    const char* s = getenv("OMP_NUM_THREADS");
    int num_threads;
    if(s){
        num_threads = atoi(s);
    }
    else{
        printf("Please define OMP_NUM_THREADS");
        exit(1);
    }

    pcg32_random_t rngs[num_threads];
    for(int i = 0; i < num_threads; i++){
        pcg32_srandom_r(&(rngs[i]), time(NULL) * (i + 1), (intptr_t)&(rngs[i]));
    }

    // start timer
    double start = omp_get_wtime();
    
    // simulation
    #pragma omp parallel for schedule(dynamic)
    for (unsigned int i = 0; i < PHOTONS ; ++i) {
        photon(rngs[omp_get_thread_num()]);
    }
    // stop timer
    double end = omp_get_wtime();
    assert(start <= end);
    double elapsed = end - start;

    // printf("# %lf seconds\n", elapsed);
    //printf("# %lf K photons per second\n", 1e-3 * PHOTONS / elapsed);
    printf("%lf\n", 1e-3 * PHOTONS / elapsed);
   /*  printf("# Radius\tHeat\n");
    printf("# [microns]\t[W/cm^3]\tError\n");
    float t = 4.0f * M_PI * powf(MICRONS_PER_SHELL, 3.0f) * PHOTONS / 1e12;
    for (unsigned int i = 0; i < SHELLS - 1; ++i) {
        printf("%6.0f\t%12.5f\t%12.5f\n", i * (float)MICRONS_PER_SHELL,
               heat[i] / t / (i * i + i + 1.0 / 3.0),
               sqrt(heat2[i] - heat[i] * heat[i] / PHOTONS) / t / (i * i + i + 1.0f / 3.0f));
    }
    printf("# extra\t%12.5f\n", heat[SHELLS - 1] / PHOTONS); */

    return 0;
}