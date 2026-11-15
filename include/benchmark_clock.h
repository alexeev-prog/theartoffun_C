#ifndef BENCHMARK_CLOCK_H
#define BENCHMARK_CLOCK_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Shared monotonic clock for the CLI suite and structured native worker. */
static inline int benchmark_clock_gettime(clockid_t clock, struct timespec *value) {
    if (clock_gettime(clock, value) != 0) {
        perror("benchmark clock_gettime");
        exit(EXIT_FAILURE);
    }
    return 0;
}

static inline uint64_t benchmark_now_ns(void) {
    struct timespec value;
    benchmark_clock_gettime(CLOCK_MONOTONIC, &value);
    return (uint64_t)value.tv_sec * UINT64_C(1000000000) + (uint64_t)value.tv_nsec;
}

#endif
