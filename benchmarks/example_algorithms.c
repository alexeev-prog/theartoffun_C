/* Reuse the actual educational sources, not copies of their algorithms.
 * Example output is consumed in memory; terminal I/O is never benchmarked. */
#include "example_algorithms.h"

#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t output_checksum;

static int consume_printf(const char* format, ...) {
    /* Retain printed values, not just call counts: otherwise an optimizer can
     * eliminate e.g. shortest-path computation. Formatting cost is included. */
    char buffer[256];
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    for (int i = 0; i < length && i < (int)sizeof(buffer) - 1; ++i) {
        output_checksum = output_checksum * 33 + (unsigned char)buffer[i];
    }
    return length;
}

#define printf consume_printf
#define main kmp_demo_main
#include "../examples/kmp.c"
#undef main
#define main boyer_demo_main
#include "../examples/boyermoore.c"
#undef main
#define main gjf_demo_main
#include "../examples/gjf_example.c"
#undef main
#define main tyche_demo_main
#include "../examples/tyche_example.c"
#undef main
#define main karatsuba_demo_main
#include "../examples/karatsuba.c"
#undef main
#define main cycle_demo_main
#include "../examples/FTaH.c"
#undef main
#define main floyd_demo_main
#include "../examples/floyd_warshall.c"
#undef main
#define main bitboard_demo_main
#include "../examples/bitboard_example.c"
#undef main
#define main fold_demo_main
#include "../examples/xor_fold.c"
#undef main
#define main xxtea_demo_main
#include "../examples/xxtea_example.c"
#undef main
#define main fuzzy_demo_main
#include "../examples/fuzzy_float_compare.c"
#undef main
#define main rsqrt_demo_main
#include "../examples/q_rsqrt.c"
#undef main
#undef printf

/* Geometry examples intentionally have separate canvases and helper names. */
#define main line_demo_main
#define buffer line_buffer
#define plot line_plot
#define clear_buffer line_clear
#define print_buffer line_print
#include "../examples/bresenham_line.c"
#undef main
#undef buffer
#undef plot
#undef clear_buffer
#undef print_buffer
#undef WIDTH
#undef HEIGHT
#define main circle_demo_main
#define buffer circle_buffer
#define plot circle_plot
#define clear_buffer circle_clear
#define print_buffer circle_print
#include "../examples/bresenham_circle.c"
#undef main
#undef buffer
#undef plot
#undef clear_buffer
#undef print_buffer

#define BATCH(name, setup, expression) \
    double example_##name(size_t size, uint64_t count, uint64_t seed) { \
        (void)size; \
        (void)seed; \
        setup; \
        double sum = 0; \
        for (uint64_t i = 0; i < count; ++i) { \
            sum += (double)(expression); \
        } \
        return sum; \
    }
BATCH(gjf64, gjf_seed(seed, seed ^ UINT64_C(0x9e3779b97f4a7c15)), gjf64())
BATCH(tyche_next, TycheState state; tyche_init(&state, (uint32_t)seed), tyche_next(&state))
BATCH(gjf_seed, (void)0, (gjf_seed(seed + i, seed + i + 1), gjf_state[0]))
BATCH(tyche_init, TycheState state, (tyche_init(&state, (uint32_t)(seed + i)), state.a))
BATCH(karatsuba, (void)0, karatsuba((long long)((i + seed) % size + 1), 1234567))
BATCH(pow10, (void)0, pow10((int)((i + seed) % 10)))
BATCH(getMax, (void)0, getMax((int)((i + seed) % size), 123))
BATCH(knights_attacks, (void)0, knights_attacks(UINT64_C(1) << ((i + seed) % 64)))
BATCH(fuzzy_float_eq, (void)0, fuzzy_float_eq((float)((i + seed) % size), 1.000001f))
BATCH(rsqrt_exact, (void)0, rsqrt_exact((float)((i + seed) % size + 1)))
BATCH(rsqrt_appr, (void)0, rsqrt_appr((float)((i + seed) % size + 1)))
BATCH(rsqrt_appr_nr, (void)0, rsqrt_appr_nr((float)((i + seed) % size + 1)))
BATCH(rsqrt_nr, (void)0, rsqrt_nr((float)((i + seed) % size + 1), rsqrt_appr((float)((i + seed) % size + 1))))

static char text[65537];
static uint32_t numbers[65536], block[65536];
static int links[65536];

static void prepare(size_t size, uint64_t seed) {
    for (size_t i = 0; i < size; ++i) {
        text[i] = (char)('a' + (i + seed) % 4);
        numbers[i] = (uint32_t)(i + seed);
        links[i] = (int)((i + 1) % size);
    }
    text[size] = 0;
}
BATCH(boyer_moore_search, prepare(size, seed), boyer_moore_search(text, "abca"))
BATCH(KMPSearch, prepare(size, seed); output_checksum = 0, (KMPSearch("abca", text), output_checksum))
BATCH(computeLPSArray, prepare(size, seed), (computeLPSArray(text, (int)size, links), links[size - 1]))
BATCH(preprocess_bad_char, int bad[256], (preprocess_bad_char("abca", 4, bad), bad[i % 256]))
BATCH(findCycle, prepare(size, seed), findCycle(links, (int)size))
BATCH(xor_fold, prepare(size, seed), xor_fold(numbers, size))
BATCH(
    xxtea_encrypt, prepare(size, seed); uint32_t key[4] = { 1 };
    key[0] = (uint32_t)seed,
    (memcpy(block, numbers, size * sizeof(*block)), xxtea_encrypt(block, (int)size, key), block[0]))
BATCH(
    xxtea_decrypt, prepare(size, seed); uint32_t key[4] = { 1 }; key[0] = (uint32_t)seed;
    xxtea_encrypt(numbers, (int)size, key),
    (memcpy(block, numbers, size * sizeof(*block)), xxtea_decrypt(block, (int)size, key), block[0]))
BATCH(
    floydWarshall, int graph[V][V];
    for (int a = 0; a < V; ++a) for (int b = 0; b < V; ++b)
        graph[a][b] = a == b ? 0 : (int)((seed + (unsigned)a + (unsigned)b) % 10 + 1);
    output_checksum = 0, (floydWarshall(graph), output_checksum))
BATCH(
    bresenham_line,
    line_clear(),
    (bresenham_line(0, 0, (int)((i + seed) % 20), 19), line_buffer[19][(i + seed) % 20]))
BATCH(
    bresenham_circle,
    circle_clear(),
    (bresenham_circle(40, 20, (int)((i + seed) % 19 + 1)), circle_buffer[20][40 + (i + seed) % 19 + 1]))
