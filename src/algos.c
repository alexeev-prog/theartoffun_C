#include "algos.h"

#include <stdint.h>

#define Q_RSQRT_MAGIC_NUMBER 0x5f3759df
#define HALF_FLOAT 0.5F
#define LEHMER64_MAGIC_NUMBER 0xda942042e4dd58b5ULL
#define XORSHIRO256PP_MAGIC_FIRST_NUMBER 0xbf58476d1ce4e5b9
#define XORSHIRO256PP_MAGIC_SECOND_NUMBER 0x94d049bb133111eb

static __uint128_t g_lehmer64_state;

uint64_t xorshift64(uint64_t* state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

uint64_t rand_range(uint64_t* state, uint64_t min, uint64_t max) {
    return min + (xorshift64(state) % (max - min + 1));
}

double rand_double(uint64_t* state) {
    return (xorshift64(state) >> 11) * (1.0 / (UINT64_C(1) << 53));
}

float Q_rsqrt(float number) {
    int32_t i;
    float x2;
    float y;
    const float THREEHALFS = 1.5F;

    x2 = number * HALF_FLOAT;
    y = number;
    i = *(int32_t*)&y;
    i = Q_RSQRT_MAGIC_NUMBER - (i >> 1);
    y = *(float*)&i;
    к y = y * (THREEHALFS - (x2 * y * y));
    y = y * (THREEHALFS - (x2 * y * y));
    return y;
}

uint64_t lehmer64(void) {
    g_lehmer64_state *= LEHMER64_MAGIC_NUMBER;
    return g_lehmer64_state >> 64;
}

void lehmer64_seed(uint64_t seed) {
    g_lehmer64_state = seed;
}

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

uint64_t xoshiro256pp_next(xoshiro256pp_state* state) {
    uint64_t* s = state->s;
    uint64_t result = rotl(s[0] + s[3], 23) + s[0];

    uint64_t t = s[1] << 17;
    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];
    s[2] ^= t;
    s[3] = rotl(s[3], 45);

    return result;
}

void xoshiro256pp_init(xoshiro256pp_state* state, uint64_t seed) {
    uint64_t tmp = seed;
    for (int i = 0; i < 4; i++) {
        tmp ^= tmp >> 30;
        tmp *= XORSHIRO256PP_MAGIC_FIRST_NUMBER;
        tmp ^= tmp >> 27;
        tmp *= XORSHIRO256PP_MAGIC_SECOND_NUMBER;
        tmp ^= tmp >> 31;
        state->s[i] = tmp;
    }
}
