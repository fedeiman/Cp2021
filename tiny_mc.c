#define _XOPEN_SOURCE 500 // M_PI

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#include "mtwister.h"
#include "params.h"
#include "wtime.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>


char t1[] = "Tiny Monte Carlo by Scott Prahl (http://omlc.ogi.edu)";
char t2[] = "1 W Point Source Heating in Infinite Isotropic Scattering Medium";
char t3[] = "CPU version, adapted for PEAGPGPU by Gustavo Castellano"
            " and Nicolas Wolovick";


// global state, heat and heat square in each shell
static float heat[SHELLS];
static float heat2[SHELLS];

inline __m256 random_vector(MTRand r) {
    return _mm256_set_ps(rand() / (float)RAND_MAX,rand() / (float)RAND_MAX,rand() / (float)RAND_MAX,rand() / (float)RAND_MAX,rand() / (float)RAND_MAX,rand() / (float)RAND_MAX,rand() / (float)RAND_MAX,rand() / (float)RAND_MAX);
}

//https://stackoverflow.com/a/39822314/9007125
//https://stackoverflow.com/a/65537754/9007125
// vectorized version of the answer by njuffa
/* natural log on [0x1.f7a5ecp-127, 0x1.fffffep127]. Maximum relative error 9.4529e-5 */
inline __m256 fast_log_sse(__m256 a){

    __m256i aInt = *(__m256i*)(&a);
    __m256i e =    _mm256_sub_epi32( aInt,  _mm256_set1_epi32(0x3f2aaaab));
            e =    _mm256_and_si256( e,  _mm256_set1_epi32(0xff800000) );
        
    __m256i subtr =  _mm256_sub_epi32(aInt, e);
    __m256 m =  *(__m256*)&subtr;

    __m256 i =  _mm256_mul_ps( _mm256_cvtepi32_ps(e), _mm256_set1_ps(1.19209290e-7f));// 0x1.0p-23
    /* m in [2/3, 4/3] */
    __m256 f =  _mm256_sub_ps( m,  _mm256_set1_ps(1.0f) );
    __m256 s =  _mm256_mul_ps(f, f); 
    /* Compute log1p(f) for f in [-1/3, 1/3] */
    __m256 r =  _mm256_fmadd_ps( _mm256_set1_ps(0.230836749f),  f,  _mm256_set1_ps(-0.279208571f) );// 0x1.d8c0f0p-3, -0x1.1de8dap-2
    __m256 t =  _mm256_fmadd_ps( _mm256_set1_ps(0.331826031f),  f,  _mm256_set1_ps(-0.498910338f) );// 0x1.53ca34p-2, -0x1.fee25ap-2
           r = _mm256_fmadd_ps(r, s, t);
           r = _mm256_fmadd_ps(r, s, t);
           r = _mm256_fmadd_ps(i, _mm256_set1_ps(0.693147182f),  r); // 0x1.62e430p-1 // log(2)
    return r;
}

/***
 * Photon
 ***/

//Working with sets of 8 at a time
static void photon()
{
    /*     
    const float albedo = MU_S * (1.0f / (MU_S + MU_A));
    const float shells_per_mfp = 1e4 * (1.0f / MICRONS_PER_SHELL) * (1.0f / (MU_A + MU_S)); 
    */

    const __m256 albedo = _mm256_set1_ps(MU_S * (1.0f / (MU_S + MU_A)));
    const __m256 shells_per_mfp = _mm256_set1_ps(1e4 * (1.0f / MICRONS_PER_SHELL) * (1.0f / (MU_A + MU_S)));
    // Move this here to avoid realocc of const value
    const __m256i shellConst = _mm256_set1_epi32(SHELLS - 1);
    const __m256 zeros = _mm256_set1_ps(0.0f);
    const __m256 ones = _mm256_set1_ps(1.0f);
    const __m256 twos = _mm256_set1_ps(2.0f);
    const __m256 pointOne = _mm256_set1_ps(0.1f);
    const __m256 pointZeroOne = _mm256_set1_ps(0.001f);
    const __m256 tens = _mm256_set1_ps(10.0f);

    /* launch */
    /*     
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float u = 0.0f;
    float v = 0.0f;
    float w = 1.0f;
    float weight = 1.0f; 
    */
    __m256 x = _mm256_set1_ps(0.0f);
    __m256 y = _mm256_set1_ps(0.0f);
    __m256 z = _mm256_set1_ps(0.0f);
    __m256 u = _mm256_set1_ps(0.0f);
    __m256 v = _mm256_set1_ps(0.0f);
    __m256 w = _mm256_set1_ps(1.0f);
    __m256 weight = _mm256_set1_ps(1.0f);
    int photon_count = 0;
    while(photon_count < PHOTONS){
        
        //float t = -logf((float)genRand(&r)); /* move */
        // Possibly not portable outside gcc, fast 
        __m256 t = -fast_log_sse(random_vector(r));
        /*
        x += t * u;
        y += t * v;
        z += t * w; 
        */
        x = _mm256_fmadd_ps(t, u, x);
        y = _mm256_fmadd_ps(t, v, y);
        z = _mm256_fmadd_ps(t, w, z);

        //unsigned int shell = sqrtf(x * x + y * y + z * z) * shells_per_mfp; /* absorb */
        __m256 shell = _mm256_mul_ps(_mm256_sqrt_ps((_mm256_fmadd_ps(x, x, _mm256_fmadd_ps(y, y, _mm256_mul_ps(z, z))))), shells_per_mfp);
        // int casting, no unsigned int, should work tho
        __m256i shell_ints =_mm256_cvtps_epi32(shell);
        shell_ints = _mm256_min_epi32(shellConst, shell_ints);
        /*
        if (shell > SHELLS - 1) {
            shell = SHELLS - 1;
        } 
        */

       __m256 heat_res = _mm256_mul_ps(_mm256_sub_ps(ones, albedo), weight);
       __m256 heat_res_squared = _mm256_mul_ps(heat_res, heat_res);

        // heat[shell] += (1.0f - albedo) * weight; 
        // heat2[shell] += (1.0f - albedo) * (1.0f - albedo) * weight * weight; /* add up squares */
        weight = _mm256_mul_ps(weight, albedo);
        //weight *= albedo;

        __m256 cond = _mm256_cmp_ps(weight, pointZeroOne, _CMP_LT_OS);
        __m256 randMask = _mm256_cmp_ps(random_vector(r), pointOne, _CMP_GT_OS);
        __m256 breakMask = _mm256_and_ps(randMask, cond);
        weight = _mm256_blendv_ps(weight, _mm256_mul_ps(weight, tens), cond);
        weight = _mm256_blendv_ps(weight, ones, breakMask);

        // You can end up with up to 7 extra photons
        for(int i = 0; i < 8; i++){
            heat[shell_ints[i]] += heat_res[i];
            heat2[shell_ints[i]] += heat_res_squared[i];

            if(weight[i] == 1.0f){
                photon_count++;
            }
        }

        // if (weight < 0.001f) { /* roulette */
        //     if ((float)genRand(&r) > 0.1f) {
        //         break;
        //     }
        //     weight *= 10.0f;
        // } 

        /* New direction, rejection method */
        __m256 xi1, xi2, condition;
        __m256 extra = breakMask;
        //float xi1, xi2;
        do {
            xi1 =  _mm256_fmsub_ps(twos, random_vector(r), ones);
            xi2 =  _mm256_fmsub_ps(twos, random_vector(r), ones);
            t = _mm256_fmadd_ps(xi1, xi1, _mm256_mul_ps(xi2, xi2));

            condition = _mm256_cmp_ps(ones, t, _CMP_GE_OS);
            extra = _mm256_or_ps(extra, condition);
            //t = xi1 * xi1 + xi2 * xi2;
            //fflush( stdout );
        } while (_mm256_movemask_ps(extra) != 255 );
        
        /*
        if ((float)genRand(&r) > 0.5) {
            xi1 = 2.0f * (float)genRand(&r) - 1.0f;
            xi2 = ((float)genRand(&r)) * 2 * sqrtf(1 - xi1 * xi1) - sqrtf(1 - xi1 * xi1);
        } else {
            xi2 = 2.0f * (float)genRand(&r) - 1.0f;
            xi1 = ((float)genRand(&r)) * 2 * sqrtf(1 - xi2 * xi2) - sqrtf(1 - xi2 * xi2);
        }
        t = xi1 * xi1 + xi2 * xi2;
        */

        //u = 2.0f * t - 1.0f;
        u = _mm256_sub_ps(_mm256_mul_ps(twos, t), ones);

        __m256 square_root = _mm256_rsqrt_ps(_mm256_div_ps(t, _mm256_sub_ps(ones, _mm256_mul_ps(u, u))));
        v = _mm256_mul_ps(xi1, square_root);
        //v = xi1 * sqrtf((1.0f - u * u) * (1.0f / t));
        w = _mm256_mul_ps(xi2, square_root);
        //w = xi2 * sqrtf((1.0f - u * u) * (1.0f / t));
        
        //weight = _mm256_blendv_ps(weight, ones, randMask );
        x = _mm256_blendv_ps(x, zeros, breakMask );
        y = _mm256_blendv_ps(y, zeros, breakMask );
        z = _mm256_blendv_ps(z, zeros, breakMask );
        u = _mm256_blendv_ps(u, zeros, breakMask );
        v = _mm256_blendv_ps(v, zeros, breakMask );
        w = _mm256_blendv_ps(w, ones, breakMask );
    }
}


/***
 * Main matter
 ***/

int main(void)
{
    printf("Working...");
    fflush( stdout );
    // heading
    /*
    printf("# %s\n# %s\n# %s\n", t1, t2, t3);
    printf("# Scattering = %8.3f/cm\n", MU_S);
    printf("# Absorption = %8.3f/cm\n", MU_A);
    printf("# Photons    = %8d\n#\n", PHOTONS);
    */
    // configure RNG
    srand(SEED);

    // start timer
    double start = wtime();
    // simulation
    photon();

    // stop timer
    double end = wtime();
    assert(start <= end);
    double elapsed = end - start;

    
    // printf("# %lf seconds\n", elapsed);
    // printf("# %lf K photons per second\n", 1e-3 * PHOTONS / elapsed);

    //
    printf("%lf\n", 1e-3 * PHOTONS / elapsed);

    
    /* printf("# Radius\tHeat\n");
    printf("# [microns]\t[W/cm^3]\tError\n");
    float t = 4.0f * M_PI * powf(MICRONS_PER_SHELL, 3.0f) * PHOTONS / 1e12;
    for (unsigned int i = 0; i < SHELLS - 1; ++i) {
        printf("%6.0f\t%12.5f\t%12.5f\n", i * (float)MICRONS_PER_SHELL,
               heat[i] / t / (i * i + i + 1.0 / 3.0),
               sqrt(heat2[i] - heat[i] * heat[i] / PHOTONS) / t / (i * i + i + 1.0f / 3.0f));
    }
    printf("# extra\t%12.5f\n", heat[SHELLS - 1] / PHOTONS);
    printf("# %lf seconds\n", elapsed);
    printf("# %lf K photons per second\n", 1e-3 * PHOTONS / elapsed); */
    return 0;
}