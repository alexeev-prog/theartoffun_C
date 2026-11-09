/* Machine-readable benchmark worker. File management belongs to run.py. */
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "algos.h"
#include "compressing.h"
#include "fib_algos.h"
#include "pow_algos.h"

static volatile double sink;
typedef double (*work_fn)(size_t, uint64_t, uint64_t);

/* Each batch has a direct call in its loop, not an indirect call per operation.
 * Inputs vary at runtime; the checksum is consumed outside the timed region.
 * Build without LTO: algorithms must remain in separate translation units. */
#define SCALAR(name, expression) \
    static double work_##name(size_t size, uint64_t count, uint64_t seed) { \
        double sum = 0; \
        for (uint64_t i = 0; i < count; ++i) { \
            uint32_t x = (uint32_t)((i + seed) % size + 1); \
            sum += (expression); \
        } \
        return sum; \
    }

SCALAR(isqrt, isqrt(x))
SCALAR(div3, div3(x))
SCALAR(to_gray, to_gray(x))
SCALAR(from_gray, from_gray(x))
SCALAR(reverse_bits, reverse_bits(x))
SCALAR(count_trailing_zeros, count_trailing_zeros(x))
SCALAR(count_trailing_zeros_kernighan, count_trailing_zeros_kernighan(x))
SCALAR(binary_pow, binary_pow(1.0 + x / (double)size, 7))
SCALAR(libm_pow, pow(1.0 + x / (double)size, 7.0))
SCALAR(fast_pow, fast_pow(1.0 + x / (double)size, 7.0))
SCALAR(fastest_pow, fastest_pow(1.0f + x / (float)size, 7.0f))
SCALAR(Q_rsqrt, Q_rsqrt((float)x))
SCALAR(libm_rsqrt, 1.0f / sqrtf((float)x))
SCALAR(basic_miles2km, basic_miles2km((float)x))
SCALAR(fib_interpolate, fib_interpolate((float)x))
SCALAR(fib_cache_convert, fib_cache_convert((float)x))
SCALAR(fib_golden_ratio, fib_golden_ratio((float)x))
SCALAR(fib_golden_ratio_binary, fib_golden_ratio_binary((float)x))
SCALAR(is_leap_year, is_leap_year(2000 + (int)(x % 400)))
SCALAR(zellers_congruence, zellers_congruence((int)(x % 28 + 1), 3, 2000 + (int)(x % 400)))

static char input[65537];
static uint8_t scratch[65536];
static char encoded[196609];

#define BUFFER(name, expression) \
    static double work_##name(size_t size, uint64_t count, uint64_t seed) { \
        double sum = 0; \
        (void)size; \
        (void)seed; \
        for (uint64_t i = 0; i < count; ++i) { sum += (expression); } \
        return sum; \
    }
BUFFER(jenkins_hash, jenkins_hash(input, size, (uint32_t)i))
BUFFER(fnv1a_hash, fnv1a_hash(input, size))
BUFFER(fletcher32_string, fletcher32_string(input))

static double work_counting_sort_256(size_t size, uint64_t count, uint64_t seed) {
    double sum = 0;
    (void)seed;
    for (uint64_t i = 0; i < count; ++i) {
        /* Restore unsorted data on EVERY operation. Copy cost is included. */
        memcpy(scratch, input, size);
        counting_sort_256(scratch, size);
        sum += scratch[i % size];
    }
    return sum;
}

static double work_rle_encode(size_t size, uint64_t count, uint64_t seed) {
    double sum = 0;
    (void)size;
    (void)seed;
    for (uint64_t i = 0; i < count; ++i) {
        rle_encode(input, encoded);
        sum += (unsigned char)encoded[0];
    }
    return sum;
}

static double work_rle_decode(size_t size, uint64_t count, uint64_t seed) {
    double sum = 0;
    (void)size;
    (void)seed;
    for (uint64_t i = 0; i < count; ++i) {
        char *decoded = rle_decode(encoded);
        if (!decoded) { fprintf(stderr, "RLE decode failed\n"); exit(1); }
        sum += (unsigned char)decoded[0];
        free(decoded);
    }
    return sum;
}

#define PRNG(name, setup, expression) \
    static double work_##name(size_t size, uint64_t count, uint64_t seed) { \
        (void)size; \
        setup; \
        uint64_t sum = 0; \
        for (uint64_t i = 0; i < count; ++i) { sum += (expression); } \
        return (double)sum; \
    }
PRNG(xorshift64, uint64_t state = seed, xorshift64(&state))
PRNG(mulberry32, uint32_t state = (uint32_t)seed, mulberry32(&state))
PRNG(pcg32, pcg32_random_t state; state.state = seed; state.inc = 1, pcg32_random_r(&state))
PRNG(xoshiro256pp, xoshiro256pp_state state; xoshiro256pp_init(&state, seed), xoshiro256pp_next(&state))
PRNG(tinymt32, tinymt32_t state; tinymt32_init(&state, (uint32_t)seed), tinymt32_generate(&state))
PRNG(lehmer64, lehmer64_seed(seed), lehmer64())

typedef struct { const char *group, *name, *axis; work_fn work; } benchmark;
#define CASE(group, name, axis) {group, #name, axis, work_##name}
static const benchmark cases[] = {
    CASE("integer", isqrt, "input_range"), CASE("integer", div3, "input_range"),
    CASE("bits", to_gray, "input_range"), CASE("bits", from_gray, "input_range"),
    CASE("bits", reverse_bits, "input_range"),
    CASE("bits", count_trailing_zeros, "input_range"),
    CASE("bits", count_trailing_zeros_kernighan, "input_range"),
    CASE("power", binary_pow, "input_range"), CASE("power", libm_pow, "input_range"),
    CASE("power", fast_pow, "input_range"), CASE("power", fastest_pow, "input_range"),
    CASE("rsqrt", Q_rsqrt, "input_range"), CASE("rsqrt", libm_rsqrt, "input_range"),
    CASE("conversion", basic_miles2km, "input_range"),
    CASE("conversion", fib_interpolate, "input_range"),
    CASE("conversion", fib_cache_convert, "input_range"),
    CASE("conversion", fib_golden_ratio, "input_range"),
    CASE("conversion", fib_golden_ratio_binary, "input_range"),
    CASE("date", is_leap_year, "input_range"),
    CASE("date", zellers_congruence, "input_range"),
    CASE("hash", jenkins_hash, "bytes"), CASE("hash", fnv1a_hash, "bytes"),
    CASE("hash", fletcher32_string, "bytes"),
    CASE("sort", counting_sort_256, "bytes"), CASE("compression", rle_encode, "bytes"),
    CASE("compression", rle_decode, "bytes"),
    CASE("prng", xorshift64, "batch_parameter"),
    CASE("prng", mulberry32, "batch_parameter"), CASE("prng", pcg32, "batch_parameter"),
    CASE("prng", xoshiro256pp, "batch_parameter"), CASE("prng", tinymt32, "batch_parameter"),
    CASE("prng", lehmer64, "batch_parameter")
};

static uint64_t number(const char *text, uint64_t max) {
    char *end;
    errno = 0;
    unsigned long long value = strtoull(text, &end, 10);
    if (errno || !*text || *text == '-' || *end || !value || value > max) {
        fprintf(stderr, "Invalid positive integer: %s\n", text);
        exit(2);
    }
    return (uint64_t)value;
}

static uint64_t now_ns(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t)) { perror("clock_gettime"); exit(1); }
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--list") == 0) {
        for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); ++i)
            printf("%s\t%s\t%s\n", cases[i].group, cases[i].name, cases[i].axis);
        return 0;
    }
    if (argc != 8) {
        fprintf(stderr, "Usage: %s --list | ALGORITHM SIZE ITERATIONS SAMPLES SEED WARMUPS 1\n", argv[0]);
        return 2;
    }
    /* argc includes a reserved protocol version to reject incompatible callers. */
    if (strcmp(argv[7], "1") != 0) return 2;
    size_t size = (size_t)number(argv[2], 65536);
    uint64_t iterations = number(argv[3], 1000000000);
    uint64_t samples = number(argv[4], 1000);
    uint64_t seed = number(argv[5], UINT32_MAX);
    uint64_t warmups = number(argv[6], 1000);
    const benchmark *selected = NULL;
    for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); ++i)
        if (strcmp(argv[1], cases[i].name) == 0) selected = &cases[i];
    if (!selected) { fprintf(stderr, "Unknown algorithm: %s\n", argv[1]); return 2; }
    uint64_t state = seed;
    for (size_t i = 0; i < size; ++i) input[i] = (char)('a' + xorshift64(&state) % 26);
    input[size] = '\0';
    rle_encode(input, encoded);
    for (uint64_t i = 0; i < warmups; ++i) sink = selected->work(size, iterations, seed);
    puts("schema_version,group,algorithm,axis,size,sample,iterations,seed,elapsed_ns,ns_per_op,checksum");
    for (uint64_t i = 0; i < samples; ++i) {
        uint64_t start = now_ns();
        double checksum = selected->work(size, iterations, seed);
        uint64_t elapsed = now_ns() - start;
        sink = checksum;
        if (!elapsed || !isfinite(checksum)) { fprintf(stderr, "Invalid measurement\n"); return 1; }
        printf("1,%s,%s,%s,%zu,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%.9f,%.17g\n",
               selected->group, selected->name, selected->axis, size, i + 1,
               iterations, seed, elapsed, (double)elapsed / iterations, checksum);
    }
    return ferror(stdout) ? 1 : 0;
}
