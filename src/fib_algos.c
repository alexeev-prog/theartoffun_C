#include <float.h>
#include <math.h>
#include <stdint.h>

#include "pow_algos.h"

#define MAX_CACHE 94

uint64_t fibonacci(int num) {
    if (num <= 0) {
        return 0;
    }

    uint64_t a = 0;
    uint64_t b = 1;

    if (num == 1) {
        return b;
    }

    for (int i = 2; i <= num; i++) {
        uint64_t next = a + b;
        a = b;
        b = next;
    }

    return b;
}

float basic_miles2km(float miles) {
    return miles * 1.609344f;
}

float fib_interpolate(float miles) {
    if (miles < 5.0F) {
        return basic_miles2km(miles);
    }

    uint64_t prev_mile = 0;
    uint64_t prev_km = 1;
    uint64_t curr_mile = 1;
    uint64_t curr_km = 2;

    while (curr_mile <= miles) {
        prev_mile = curr_mile;
        prev_km = curr_km;

        curr_mile = prev_km;
        curr_km = prev_mile + prev_km;

        if (curr_km < prev_km || curr_mile < prev_mile) {
            break;
        }
    }

    return prev_km + ((miles - prev_mile) * ((float)(curr_km - prev_km) / (curr_mile - prev_mile)));
}

float fib_cache_convert(float miles) {
    static uint64_t cache[MAX_CACHE] = { 0 };
    static int initialized = 0;

    if (!initialized) {
        cache[0] = 0;
        cache[1] = 1;
        for (int i = 2; i < MAX_CACHE; i++) {
            cache[i] = cache[i - 1] + cache[i - 2];
        }
        initialized = 1;
    }

    if (miles < 5.0F) {
        return basic_miles2km(miles);
    }

    int i = 2;
    while (i < MAX_CACHE - 2 && cache[i] <= miles) {
        i++;
    }

    if (i >= MAX_CACHE - 2) {
        return basic_miles2km(miles);
    }

    uint64_t Fn = cache[i - 1];
    uint64_t Fn1 = cache[i];
    uint64_t Fn2 = cache[i + 1];

    return Fn1 + ((miles - Fn) * ((float)(Fn2 - Fn1) / (Fn1 - Fn)));
}

float fib_golden_ratio(float miles) {
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;

    if (miles < 1e-5) {
        return 0.0F;
    }

    double n = log(miles * sqrt(5.0)) / log(PHI);
    int k = (int)floor(n);

    double Fk = (pow(PHI, k) - pow(-PHI, -k)) / sqrt(5.0);
    double Fk1 = (pow(PHI, k + 1) - pow(-PHI, -k - 1)) / sqrt(5.0);
    double Fk2 = (pow(PHI, k + 2) - pow(-PHI, -k - 2)) / sqrt(5.0);

    if (Fk1 - Fk < DBL_EPSILON) {
        return basic_miles2km(miles);
    }

    return Fk1 + ((miles - Fk) * ((float)(Fk2 - Fk1) / (Fk1 - Fk)));
}

float fib_golden_ratio_binary(float miles) {
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;

    if (miles < 1e-5) {
        return 0.0F;
    }

    double n = log(miles * sqrt(5.0)) / log(PHI);
    int k = (int)floor(n);

    double sign_k = (k % 2 == 0) ? 1.0 : -1.0;
    double sign_k1 = ((k + 1) % 2 == 0) ? 1.0 : -1.0;
    double sign_k2 = ((k + 2) % 2 == 0) ? 1.0 : -1.0;

    double phi_k = binary_pow(PHI, k);
    double phi_k1 = binary_pow(PHI, k + 1);
    double phi_k2 = binary_pow(PHI, k + 2);

    double Fk = (phi_k - sign_k / phi_k) / sqrt(5.0);
    double Fk1 = (phi_k1 - sign_k1 / phi_k1) / sqrt(5.0);
    double Fk2 = (phi_k2 - sign_k2 / phi_k2) / sqrt(5.0);

    if (Fk1 - Fk < DBL_EPSILON) {
        return basic_miles2km(miles);
    }

    return Fk1 + ((miles - Fk) * ((float)(Fk2 - Fk1) / (Fk1 - Fk)));
}
