#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 4096
#define T 1000
#define E9 1000000000

#ifndef CLOCK_REALTIME
#    define CLOCK_REALTIME 0
#endif

enum methods
{
    EXACT,
    APPR,
    APPR_NR,
    M
};

const char* METHODS[] = {"exact", "appr", "appr_nr"};

static inline float rsqrt_exact(float x) {
    return 1.0f / sqrtf(x);
}

static inline float rsqrt_appr(float x) {
    uint32_t i = *(uint32_t*)&x;
    i = -(i >> 1) + 0x5f3759df;
    return *(float*)&i;
}

static inline float rsqrt_nr(float x, float y) {
    return y * (1.5f - x * 0.5f * y * y);
}

static inline float rsqrt_appr_nr(float x) {
    float y = rsqrt_appr(x);
    return rsqrt_nr(x, y);
}

int main() {
    srand(time(NULL));

    float y_sum[M] = {0};
    double t[M] = {0};

    for (int trial = 0; trial < T; trial++) {
        struct timespec start, stop;
        float x[N], y[N];
        for (int i = 0; i < N; i++) {
            x[i] = rand();
        }

        clock_gettime(CLOCK_REALTIME, &start);
        for (int i = 0; i < N; i++) {
            y[i] = rsqrt_exact(x[i]);
        }
        clock_gettime(CLOCK_REALTIME, &stop);
        for (int i = 0; i < N; i++) {
            y_sum[EXACT] += y[i];
        }
        t[EXACT] += ((stop.tv_sec - start.tv_sec) * E9 + stop.tv_nsec - start.tv_nsec);

        clock_gettime(CLOCK_REALTIME, &start);
        for (int i = 0; i < N; i++) {
            y[i] = rsqrt_appr(x[i]);
        }
        clock_gettime(CLOCK_REALTIME, &stop);
        for (int i = 0; i < N; i++) {
            y_sum[APPR] += y[i];
        }
        t[APPR] += ((stop.tv_sec - start.tv_sec) * E9 + stop.tv_nsec - start.tv_nsec);

        clock_gettime(CLOCK_REALTIME, &start);
        for (int i = 0; i < N; i++) {
            y[i] = rsqrt_appr_nr(x[i]);
        }
        clock_gettime(CLOCK_REALTIME, &stop);
        for (int i = 0; i < N; i++) {
            y_sum[APPR_NR] += y[i];
        }
        t[APPR_NR] += ((stop.tv_sec - start.tv_sec) * E9 + stop.tv_nsec - start.tv_nsec);
    }

    printf("rsqrt\tps/op\tratio\terr\n");
    for (int m = 0; m < M; m++) {
        printf("%s\t%.0f\t%.2f\t%.4f\n",
               METHODS[m],
               t[m] * 1000.0f / N / T,
               (double)t[EXACT] / t[m],
               (y_sum[m] - y_sum[EXACT]) / y_sum[EXACT]);
    }

    return 0;
}
