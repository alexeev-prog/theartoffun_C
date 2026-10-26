#include "pow_algos.h"

#include <limits.h>
#include <math.h>

#define FAST_POW_MAGIC_NUMBER 1072632447
#define FASTEST_POW_MAGIC_NUMBER 1065353210

double binary_pow(double b, unsigned long long e) {
    double v = 1.0;
    while (e != 0) {
        if ((e & 1) != 0) {
            v *= b;
        }
        b *= b;
        e >>= 1;
    }
    return v;
}

double fast_pow(double a_coeff, double base) {
    if (!isfinite(a_coeff) || a_coeff <= 0 || !isfinite(base)) {
        return NAN;
    }

    union {
        double d;
        int x[2];
    } u = { a_coeff };

    double bits = base * ((double)u.x[1] - FAST_POW_MAGIC_NUMBER) + FAST_POW_MAGIC_NUMBER;
    if (!isfinite(bits) || bits < 0 || bits >= 0x7ff00000) {
        return NAN;
    }
    u.x[1] = (int)bits;
    u.x[0] = 0;
    return u.d;
}

float fastest_pow(float a_coeff, float base) {
    if (!isfinite(a_coeff) || a_coeff <= 0 || !isfinite(base)) {
        return NAN;
    }

    union {
        float d;
        int x;
    } u = { a_coeff };

    double bits = (double)base * ((double)u.x - FASTEST_POW_MAGIC_NUMBER) + FASTEST_POW_MAGIC_NUMBER;
    if (!isfinite(bits) || bits < 0 || bits >= 0x7f800000) {
        return NAN;
    }
    u.x = (int)bits;

    return u.d;
}

int8_t is_power_of_two(uint32_t x) {
    return x && !(x & (x - 1));
}
