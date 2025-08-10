#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdint.h>

typedef struct {
    uint64_t s[4];
} xoshiro256pp_state;

uint64_t xorshift64(uint64_t* state);

uint64_t rand_range(uint64_t* state, uint64_t min, uint64_t max);

double rand_double(uint64_t* state);

float Q_rsqrt(float number);

uint64_t lehmer64(void);

void lehmer64_seed(uint64_t seed);

uint64_t xoshiro256pp_next(xoshiro256pp_state* state);

void xoshiro256pp_init(xoshiro256pp_state* state, uint64_t seed);

#endif
