#include "test.cu"

char t1[] = "Tiny Monte Carlo by Scott Prahl (http://omlc.ogi.edu)";
char t2[] = "1 W Point Source Heating in Infinite Isotropic Scattering Medium";
char t3[] = "CPU version, adapted for PEAGPGPU by Gustavo Castellano"
            " and Nicolas Wolovick";
char error[] = "Error";



int main()
{

    if (verbose) { // heading
        printf("# %s\n# %s\n# %s\n", t1, t2, t3);
        printf("# Scattering = %8.3f/cm\n", MU_S);
        printf("# Absorption = %8.3f/cm\n", MU_A);
        printf("# Photons    = %8d\n#\n", PHOTONS);
    }

    float ** heat;

    if (cudaMallocManaged(&heat, SHELLS * sizeof(float *)) != cudaSuccess) return printf("# %s\n#", error);
    for (int i = 0; i < SHELLS; i++) {
        if(cudaMallocManaged(&heat[i], 2 * sizeof(float))!= cudaSuccess ) return printf("# %s\n#", error);
    }

    (void)run_gpu_tiny_mc(heat, PHOTONS);

    for (int i = 0; i < SHELLS; i++) {
        heat[i][0] = heat[i][1] = 0.0f;
    }


    double start = wtime();
    (void)run_gpu_tiny_mc(heat, PHOTONS);
    double end = wtime();
    assert(start <= end);
    double elapsed = (end - start);

    printf("%lf\n", 1e-3 * PHOTONS / elapsed);



    return 0;
}