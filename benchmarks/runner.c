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
#include "example_algorithms.h"
#include "fib_algos.h"
#include "levenshtein.h"
#include "mortoncode.h"
#include "pow_algos.h"

static volatile double sink;
typedef double (*work_fn)(size_t, uint64_t, uint64_t);
typedef double (*value_fn)(uint32_t, size_t);
static uint32_t values[65536];

/* Each batch has a direct call in its loop, not an indirect call per operation.
 * Inputs vary at runtime; the checksum is consumed outside the timed region.
 * Build without LTO: algorithms must remain in separate translation units. */
#define SCALAR(name, expression) \
    static inline double value_##name(uint32_t x, size_t size) { \
        (void)size; \
        return (expression); \
    } \
    static double work_##name(size_t size, uint64_t count, uint64_t seed) { \
        double sum = 0; \
        for (uint64_t i = 0; i < count; ++i) { \
            uint32_t x = values[(i + seed) % size]; \
            sum += value_##name(x, size); \
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
SCALAR(fibonacci, fibonacci((int)(x % 94)))
SCALAR(fast_mod, fast_mod(x, 16))
SCALAR(is_power_of_two, is_power_of_two(x))
SCALAR(next_power_of_two, next_power_of_two(x))
SCALAR(morton_encode, morton_encode((uint16_t)x, (uint16_t)(x ^ 0xaaaa)))
SCALAR(calculate_pi_leibniz, calculate_pi_leibniz(x))

static double value_morton_decode(uint32_t x, size_t size) {
    uint16_t a, b;
    (void)size;
    morton_decode(x, &a, &b);
    return a + b;
}

static double work_morton_decode(size_t size, uint64_t count, uint64_t seed) {
    double sum = 0;
    for (uint64_t i = 0; i < count; ++i) {
        sum += value_morton_decode(values[(i + seed) % size], size);
    }
    return sum;
}

static char input[65537];
static uint8_t scratch[65536];
static char encoded[196609];
static char second_input[65537];
static uint16_t words[65536];
static uint32_t shuffled[65536];

#define BUFFER(name, expression) \
    static double work_##name(size_t size, uint64_t count, uint64_t seed) { \
        double sum = 0; \
        (void)size; \
        (void)seed; \
        for (uint64_t i = 0; i < count; ++i) { \
            sum += (expression); \
        } \
        return sum; \
    }
BUFFER(jenkins_hash, jenkins_hash(input, size, (uint32_t)i))
BUFFER(fnv1a_hash, fnv1a_hash(input, size))
BUFFER(fletcher32_string, fletcher32_string(input))
BUFFER(fletcher32, fletcher32(words, size))
BUFFER(is_palindrome_bit, is_palindrome_bit(input))

static double work_levenshtein(size_t size, uint64_t count, uint64_t seed) {
    double sum = 0;
    (void)seed;
    for (uint64_t i = 0; i < count; ++i) {
        int result = levenshtein(input, second_input);
        if (result < 0) {
            fprintf(stderr, "Levenshtein allocation failed\n");
            exit(1);
        }
        sum += result;
    }
    (void)size;
    return sum;
}

static double work_fisher_yates_shuffle(size_t size, uint64_t count, uint64_t seed) {
    double sum = 0;
    for (uint64_t i = 0; i < count; ++i) {
        memcpy(shuffled, values, size * sizeof(*values));
        fisher_yates_shuffle(shuffled, size, &seed);
        sum += shuffled[i % size];
    }
    return sum;
}

#define MUTATE(name, setup, operation, result) \
    static double work_##name(size_t size, uint64_t count, uint64_t seed) { \
        (void)size; \
        (void)seed; \
        setup; \
        double sum = 0; \
        for (uint64_t i = 0; i < count; ++i) { \
            operation; \
            sum += (double)(result); \
        } \
        return sum; \
    }
MUTATE(xor_swap, int a = 42; int b = (int)seed, xor_swap(&a, &b), a)
MUTATE(
    jenkins_mix, uint32_t a = (uint32_t)seed; uint32_t b = 17;
    uint32_t c = 29, jenkins_mix(&a, &b, &c), a ^ b ^ c)
MUTATE(
    jenkins_final, uint32_t a = (uint32_t)seed; uint32_t b = 17;
    uint32_t c = 29, jenkins_final(&a, &b, &c), a ^ b ^ c)
MUTATE(
    xor_list_add, xor_node a = { 0 }; xor_node b = { 0 }; xor_node c = { 0 }, a.link = (uintptr_t)&c;
    c.link = (uintptr_t)&a;
    xor_list_add(&a, &b, &c), (a.link == (uintptr_t)&b) + (c.link == (uintptr_t)&b))
MUTATE(
    xor_list_next, xor_node a = { 0 }; xor_node b = { 0 }; xor_node c = { 0 };
    b.link = (uintptr_t)&a ^ (uintptr_t)&c, (void)0, xor_list_next(&a, &b) == &c)

MUTATE(xoshiro256pp_init, xoshiro256pp_state state, xoshiro256pp_init(&state, seed + i), state.s[0])
MUTATE(tinymt32_init, tinymt32_t state, tinymt32_init(&state, (uint32_t)(seed + i)), state.state[0])
MUTATE(murmur3_prng_init, murmur3_prng_t state, murmur3_prng_init(&state, seed + i), state.seed)
MUTATE(lehmer64_seed, (void)0, lehmer64_seed(seed + i), i)
MUTATE(
    rc4_init, RC4_ctx state; uint8_t key[8] = { 1 };
    key[0] = (uint8_t)seed, rc4_init(&state, key, 8), state.S[i % 256])

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

#define DECODE(name, expression) \
    static double work_##name(size_t size, uint64_t count, uint64_t seed) { \
        double sum = 0; \
        (void)size; \
        (void)seed; \
        for (uint64_t i = 0; i < count; ++i) { \
            char* decoded = (expression); \
            if (!decoded) { \
                fprintf(stderr, "RLE decode failed\n"); \
                exit(1); \
            } \
            sum += (unsigned char)decoded[0]; \
            free(decoded); \
        } \
        return sum; \
    }
DECODE(rle_decode, rle_decode(encoded))
DECODE(rle_decode_limit, rle_decode_limit(encoded, size))

static double work_rand_double(size_t size, uint64_t count, uint64_t seed) {
    double sum = 0;
    (void)size;
    for (uint64_t i = 0; i < count; ++i) {
        sum += rand_double(&seed);
    }
    return sum;
}

#define PRNG(name, setup, expression) \
    static double work_##name(size_t size, uint64_t count, uint64_t seed) { \
        (void)size; \
        setup; \
        uint64_t sum = 0; \
        for (uint64_t i = 0; i < count; ++i) { \
            sum += (expression); \
        } \
        return (double)sum; \
    }
PRNG(xorshift64, uint64_t state = seed, xorshift64(&state))
PRNG(mulberry32, uint32_t state = (uint32_t)seed, mulberry32(&state))
PRNG(pcg32, pcg32_random_t state; state.state = seed; state.inc = 1, pcg32_random_r(&state))
PRNG(xoshiro256pp, xoshiro256pp_state state; xoshiro256pp_init(&state, seed), xoshiro256pp_next(&state))
PRNG(tinymt32, tinymt32_t state; tinymt32_init(&state, (uint32_t)seed), tinymt32_generate(&state))
PRNG(lehmer64, lehmer64_seed(seed), lehmer64())
PRNG(rand_range, uint64_t state = seed, rand_range(&state, 1, 1000))
PRNG(micro_rand, uint8_t state = (uint8_t)seed, micro_rand(&state))
PRNG(murmur3_prng_next, murmur3_prng_t state; murmur3_prng_init(&state, seed), murmur3_prng_next(&state))
PRNG(
    sha1_prng, uint32_t state[16];
    for (int j = 0; j < 16; ++j) state[j] = (uint32_t)seed + (uint32_t)j, sha1_prng(state))
PRNG(
    rc4_byte, RC4_ctx state; uint8_t key[8] = { 1 }; key[0] = (uint8_t)seed;
    rc4_init(&state, key, 8), rc4_byte(&state))

/* Historical generators expose global state. Reset it outside algorithm source
 * via the public reset API; every calibration/warmup/sample batch is repeatable. */
PRNG(wyrand, legacy_prng_seed(seed), wyrand())
PRNG(msws32, legacy_prng_seed(seed), msws32())
PRNG(romu_duo, legacy_prng_seed(seed), romu_duo())
PRNG(sfc32, legacy_prng_seed(seed), sfc32())
PRNG(jsf32, legacy_prng_seed(seed), jsf32())
PRNG(ranq1, legacy_prng_seed(seed), ranq1())
MUTATE(legacy_prng_seed, (void)0, legacy_prng_seed(seed + i), i)

static double reference_power(uint32_t x, size_t size) {
    return pow(1.0 + x / (double)size, 7.0);
}

static double reference_rsqrt(uint32_t x, size_t size) {
    (void)size;
    return 1.0 / sqrt((double)x);
}

static double reference_conversion(uint32_t x, size_t size) {
    (void)size;
    return x * 1.609344;
}

typedef struct {
    const char *group, *name, *axis;
    work_fn work;
    value_fn value, reference;
    unsigned max_size;
    int example;
} benchmark;

#define CASE(group, name, axis) { group, #name, axis, work_##name, NULL, NULL, 65536, 0 }
#define ACC(group, name) \
    { #group, #name, "input_range", work_##name, value_##name, reference_##group, 65536, 0 }
static const benchmark cases[] = { CASE("integer", isqrt, "input_range"),
                                   CASE("integer", div3, "input_range"),
                                   CASE("bits", to_gray, "input_range"),
                                   CASE("bits", from_gray, "input_range"),
                                   CASE("bits", reverse_bits, "input_range"),
                                   CASE("bits", count_trailing_zeros, "input_range"),
                                   CASE("bits", count_trailing_zeros_kernighan, "input_range"),
                                   ACC(power, binary_pow),
                                   ACC(power, libm_pow),
                                   ACC(power, fast_pow),
                                   ACC(power, fastest_pow),
                                   ACC(rsqrt, Q_rsqrt),
                                   ACC(rsqrt, libm_rsqrt),
                                   ACC(conversion, basic_miles2km),
                                   ACC(conversion, fib_interpolate),
                                   ACC(conversion, fib_cache_convert),
                                   ACC(conversion, fib_golden_ratio),
                                   ACC(conversion, fib_golden_ratio_binary),
                                   CASE("date", is_leap_year, "input_range"),
                                   CASE("date", zellers_congruence, "input_range"),
                                   CASE("hash", jenkins_hash, "bytes"),
                                   CASE("hash", fnv1a_hash, "bytes"),
                                   CASE("hash", fletcher32_string, "bytes"),
                                   CASE("sort", counting_sort_256, "bytes"),
                                   CASE("compression", rle_encode, "bytes"),
                                   CASE("compression", rle_decode, "bytes"),
                                   CASE("prng", xorshift64, "batch_parameter"),
                                   CASE("prng", mulberry32, "batch_parameter"),
                                   CASE("prng", pcg32, "batch_parameter"),
                                   CASE("prng", xoshiro256pp, "batch_parameter"),
                                   CASE("prng", tinymt32, "batch_parameter"),
                                   CASE("prng", lehmer64, "batch_parameter"),
                                   CASE("integer", fibonacci, "input_range"),
                                   CASE("integer", fast_mod, "input_range"),
                                   CASE("bits", is_power_of_two, "input_range"),
                                   CASE("bits", next_power_of_two, "input_range"),
                                   CASE("morton", morton_encode, "input_range"),
                                   CASE("morton", morton_decode, "input_range"),
                                   CASE("pi", calculate_pi_leibniz, "input_range"),
                                   CASE("string", levenshtein, "bytes"),
                                   CASE("string", is_palindrome_bit, "bytes"),
                                   CASE("hash", fletcher32, "words"),
                                   CASE("hash", jenkins_mix, "batch_parameter"),
                                   CASE("hash", jenkins_final, "batch_parameter"),
                                   CASE("shuffle", fisher_yates_shuffle, "elements"),
                                   CASE("swap", xor_swap, "batch_parameter"),
                                   CASE("xor_list", xor_list_add, "batch_parameter"),
                                   CASE("xor_list", xor_list_next, "batch_parameter"),
                                   CASE("compression", rle_decode_limit, "bytes"),
                                   CASE("prng", rand_range, "batch_parameter"),
                                   CASE("prng", rand_double, "batch_parameter"),
                                   CASE("prng", micro_rand, "batch_parameter"),
                                   CASE("prng", murmur3_prng_next, "batch_parameter"),
                                   CASE("prng", sha1_prng, "batch_parameter"),
                                   CASE("prng", rc4_byte, "batch_parameter"),
                                   CASE("prng", wyrand, "batch_parameter"),
                                   CASE("prng", msws32, "batch_parameter"),
                                   CASE("prng", romu_duo, "batch_parameter"),
                                   CASE("prng", sfc32, "batch_parameter"),
                                   CASE("prng", jsf32, "batch_parameter"),
                                   CASE("prng", ranq1, "batch_parameter"),
                                   CASE("prng_init", xoshiro256pp_init, "batch_parameter"),
                                   CASE("prng_init", tinymt32_init, "batch_parameter"),
                                   CASE("prng_init", murmur3_prng_init, "batch_parameter"),
                                   CASE("prng_init", rc4_init, "batch_parameter"),
                                   CASE("prng_init", lehmer64_seed, "batch_parameter"),
                                   CASE("prng_init", legacy_prng_seed, "batch_parameter")
#define EXAMPLE(name, axis, limit) , { "examples", #name, axis, example_##name, NULL, NULL, limit, 1 }
                                       EXAMPLE_ALGORITHMS(EXAMPLE)
#undef EXAMPLE
};

static uint64_t number(const char* text, uint64_t max) {
    char* end;
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
    if (clock_gettime(CLOCK_MONOTONIC, &t)) {
        perror("clock_gettime");
        exit(1);
    }
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

int main(int argc, char** argv) {
    if (argc == 2 && strcmp(argv[1], "--catalog") == 0) {
        for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); ++i) {
            printf(
                "%s\t%s\t%s\t%s\t%d\t%u\n",
                cases[i].group,
                cases[i].name,
                cases[i].axis,
                cases[i].example || strcmp(cases[i].axis, "batch_parameter") == 0
                    ? "default"
                    : "random,sorted,reverse,repeated,boundary",
                cases[i].reference != NULL,
                strcmp(cases[i].name, "levenshtein") == 0 ? 256u : cases[i].max_size);
        }
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--list") == 0) {
        for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); ++i) {
            printf("%s\t%s\t%s\n", cases[i].group, cases[i].name, cases[i].axis);
        }
        return 0;
    }
    if (argc != 8 && argc != 11) {
        fprintf(stderr, "Usage: %s --list | ALGORITHM SIZE ITERATIONS SAMPLES SEED WARMUPS 1\n", argv[0]);
        return 2;
    }
    /* argc includes a reserved protocol version to reject incompatible callers. */
    int modern = argc == 11;
    if (strcmp(argv[7], modern ? "2" : "1") != 0) {
        return 2;
    }
    size_t size = (size_t)number(argv[2], 65536);
    uint64_t iterations = number(argv[3], 1000000000);
    uint64_t samples = number(argv[4], 1000);
    uint64_t seed = number(argv[5], UINT32_MAX);
    uint64_t warmups = number(argv[6], 1000);
    const benchmark* selected = NULL;
    for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); ++i) {
        if (strcmp(argv[1], cases[i].name) == 0) {
            selected = &cases[i];
        }
    }
    if (!selected) {
        fprintf(stderr, "Unknown algorithm: %s\n", argv[1]);
        return 2;
    }
    if (size > selected->max_size) {
        fprintf(stderr, "Size exceeds catalog limit\n");
        return 2;
    }
    if (strcmp(selected->name, "levenshtein") == 0 && size > 256) {
        fprintf(stderr, "Levenshtein benchmark size limit: 256\n");
        return 2;
    }
    const char* scenario = modern ? argv[8] : "random";
    const char* scenarios[] = { "random", "sorted", "reverse", "repeated", "boundary", "default" };
    int scenario_id = -1;
    for (int i = 0; i < 6; ++i) {
        if (strcmp(scenario, scenarios[i]) == 0) {
            scenario_id = i;
        }
    }
    if (scenario_id < 0
        || (modern
            && ((selected->example || strcmp(selected->axis, "batch_parameter") == 0)
                != (scenario_id == 5)))) {
        return 2;
    }
    uint64_t target = modern ? number(argv[9], 10000) * UINT64_C(1000000) : 0;
    if (modern && strcmp(argv[10], "timing") != 0 && strcmp(argv[10], "accuracy") != 0) {
        return 2;
    }
    uint64_t state = seed;
    for (size_t i = 0; i < size; ++i) {
        uint64_t random = xorshift64(&state);
        values[i] = (uint32_t)(random % size + 1);
        input[i] = (char)('a' + random % 26);
        if (scenario_id == 1 || scenario_id == 2) {
            size_t rank = scenario_id == 1 ? i : size - i - 1;
            values[i] = (uint32_t)rank + 1;
            input[i] = (char)('a' + rank * 26 / size);
        } else if (scenario_id == 3) {
            values[i] = (uint32_t)(size / 2 + 1);
            input[i] = 'a';
        } else if (scenario_id == 4) {
            values[i] = i % 2 ? (uint32_t)size : 1;
            input[i] = i % 2 ? 'z' : 'a';
        }
        words[i] = (uint16_t)values[i];
    }
    input[size] = '\0';
    memcpy(second_input, input, size + 1);
    for (size_t i = 0; i < size; i += 7) {
        second_input[i] = input[i] == 'z' ? 'a' : (char)(input[i] + 1);
    }
    rle_encode(input, encoded);
    if (modern && strcmp(argv[10], "accuracy") == 0) {
        if (!selected->reference) {
            return 2;
        }
        puts(
            "schema_version,group,algorithm,scenario,size,point,input,actual,reference,absolute_error,"
            "relative_error");
        /* Probe every generated input, bounded by the same size limit as timing. */
        for (size_t i = 0; i < size; ++i) {
            double actual = selected->value(values[i], size);
            double reference = selected->reference(values[i], size);
            double absolute = fabs(actual - reference);
            if (!isfinite(actual) || !isfinite(reference)) {
                return 1;
            }
            printf(
                "2,%s,%s,%s,%zu,%zu,%u,%.17g,%.17g,%.17g,",
                selected->group,
                selected->name,
                scenario,
                size,
                i + 1,
                values[i],
                actual,
                reference,
                absolute);
            if (reference != 0) {
                printf("%.17g", absolute / fabs(reference));
            }
            puts("");
        }
        return ferror(stdout) ? 1 : 0;
    }
    if (modern) {
        for (int attempt = 0; attempt < 12; ++attempt) {
            uint64_t start = now_ns();
            sink = selected->work(size, iterations, seed);
            uint64_t elapsed = now_ns() - start;
            if (elapsed >= target || iterations == 1000000000) {
                break;
            }
            double factor = fmin(10.0, fmax(2.0, (double)target / (double)(elapsed ? elapsed : 1) * 1.1));
            iterations = (uint64_t)fmin(1000000000.0, ceil(iterations * factor));
        }
    }
    for (uint64_t i = 0; i < warmups; ++i) {
        sink = selected->work(size, iterations, seed);
    }
    printf(
        "schema_version,group,algorithm,axis,size,sample,iterations,seed,elapsed_ns,ns_per_op,checksum%s\n",
        modern ? ",scenario" : "");
    for (uint64_t i = 0; i < samples; ++i) {
        uint64_t start = now_ns();
        double checksum = selected->work(size, iterations, seed);
        uint64_t elapsed = now_ns() - start;
        sink = checksum;
        if (!elapsed || !isfinite(checksum)) {
            fprintf(stderr, "Invalid measurement\n");
            return 1;
        }
        printf(
            "%d,%s,%s,%s,%zu,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%.9f,%.17g",
            modern ? 2 : 1,
            selected->group,
            selected->name,
            selected->axis,
            size,
            i + 1,
            iterations,
            seed,
            elapsed,
            (double)elapsed / iterations,
            checksum);
        if (modern) {
            printf(",%s", scenario);
        }
        puts("");
    }
    return ferror(stdout) ? 1 : 0;
}
