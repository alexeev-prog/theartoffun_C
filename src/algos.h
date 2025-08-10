#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdint.h>

uint64_t xorshift64(uint64_t* state);

uint64_t rand_range(uint64_t* state, uint64_t min, uint64_t max);

double rand_double(uint64_t* state);

float Q_rsqrt(float number);

#endif
