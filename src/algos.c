#include "algos.h"

#include <stdint.h>
#include <stdlib.h>

#define Q_RSQRT_MAGIC_NUMBER 0x5f3759df
#define HALF_FLOAT 0.5F
#define LEHMER64_MAGIC_NUMBER 0xda942042e4dd58b5ULL
#define XORSHIRO256PP_MAGIC_FIRST_NUMBER 0xbf58476d1ce4e5b9
#define XORSHIRO256PP_MAGIC_SECOND_NUMBER 0x94d049bb133111eb
#define DIV3_MAGIC_NUMBER 0xAAAAAAABULL

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

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
    y = y * (THREEHALFS - (x2 * y * y));
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

uint32_t div3(uint32_t x) {
    return (uint32_t)(((uint64_t)x * 0xAAAAAAABULL) >> 33);
}

void xor_swap(int* a, int* b) {
    if (a != b) {
        *a ^= *b;
        *b ^= *a;
        *a ^= *b;
    }
}

int count_trailing_zeros(unsigned int x) {
    if (x == 0) {
        return sizeof(x) * 8;
    }

    int count = 0;
    while ((x & 1) == 0) {
        count++;
        x >>= 1;
    }
    return count;
}

int count_trailing_zeros_kernighan(unsigned int x) {
    if (x == 0) {
        return sizeof(x) * 8;
    }

    unsigned int lowest_set_bit = x & -x;

    int count = 0;
    while (lowest_set_bit > 1) {
        count++;
        lowest_set_bit >>= 1;
    }
    return count;
}

uint32_t pcg32_random_r(pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void jenkins_mix(uint32_t* a, uint32_t* b, uint32_t* c) {
    *a -= *c;
    *a ^= rot(*c, 4);
    *c += *b;
    *b -= *a;
    *b ^= rot(*a, 6);
    *a += *c;
    *c -= *b;
    *c ^= rot(*b, 8);
    *b += *a;
    *a -= *c;
    *a ^= rot(*c, 16);
    *c += *b;
    *b -= *a;
    *b ^= rot(*a, 19);
    *a += *c;
    *c -= *b;
    *c ^= rot(*b, 4);
    *b += *a;
}

void jenkins_final(uint32_t* a, uint32_t* b, uint32_t* c) {
    *c ^= *b;
    *c -= rot(*b, 14);
    *a ^= *c;
    *a -= rot(*c, 11);
    *b ^= *a;
    *b -= rot(*a, 25);
    *c ^= *b;
    *c -= rot(*b, 16);
    *a ^= *c;
    *a -= rot(*c, 4);
    *b ^= *a;
    *b -= rot(*a, 14);
    *c ^= *b;
    *c -= rot(*b, 24);
}

uint32_t jenkins_hash(const void* key, size_t length, uint32_t initval) {
    uint32_t a, b, c;
    const uint8_t* k = (const uint8_t*)key;
    uint32_t* blocks = (uint32_t*)k;

    a = b = c = 0xdeadbeef + ((uint32_t)length) + initval;

    size_t len = length;
    while (len >= 12) {
        a += blocks[0];
        b += blocks[1];
        c += blocks[2];
        jenkins_mix(&a, &b, &c);
        blocks += 3;
        len -= 12;
    }

    uint8_t* tail = (uint8_t*)blocks;
    switch (len) {
        case 11:
            c += ((uint32_t)tail[10]) << 16;
        case 10:
            c += ((uint32_t)tail[9]) << 8;
        case 9:
            c += tail[8];
        case 8:
            b += ((uint32_t)tail[7]) << 24;
        case 7:
            b += ((uint32_t)tail[6]) << 16;
        case 6:
            b += ((uint32_t)tail[5]) << 8;
        case 5:
            b += tail[4];
        case 4:
            a += ((uint32_t)tail[3]) << 24;
        case 3:
            a += ((uint32_t)tail[2]) << 16;
        case 2:
            a += ((uint32_t)tail[1]) << 8;
        case 1:
            a += tail[0];
    }

    jenkins_final(&a, &b, &c);

    return c;
}

int8_t is_power_of_two(uint32_t x) {
    return x && !(x & (x - 1));
}

uint32_t fast_mod(uint32_t x, uint32_t mod) {
    return x & (mod - 1);
}
