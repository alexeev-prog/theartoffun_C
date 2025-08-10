#include <stdint.h>

uint64_t xorshift64(uint64_t* state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

uint64_t rand_range(uint64_t* state, uint64_t min, uint64_t max) {
    return min + xorshift64(state) % (max - min + 1);
}

double rand_double(uint64_t* state) {
    return (xorshift64(state) >> 11) * (1.0 / (UINT64_C(1) << 53));
}

float Q_rsqrt(float number) {
    int32_t i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *(int32_t*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y));
    y = y * (threehalfs - (x2 * y * y));
    return y;
}
