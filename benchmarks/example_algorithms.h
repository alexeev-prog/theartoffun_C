#ifndef BENCHMARK_EXAMPLE_ALGORITHMS_H
#define BENCHMARK_EXAMPLE_ALGORITHMS_H
#include <stddef.h>
#include <stdint.h>
/* One registry reused for declarations and benchmark registration. Arguments:
 * original function name, workload axis, maximum size. */
#define EXAMPLE_ALGORITHMS(X) \
    X(gjf64, "batch_parameter", 65536) \
    X(gjf_seed, "batch_parameter", 65536) \
    X(tyche_next, "batch_parameter", 65536) \
    X(tyche_init, "batch_parameter", 65536) \
    X(karatsuba, "input_range", 65536) \
    X(pow10, "input_range", 65536) \
    X(getMax, "input_range", 65536) \
    X(knights_attacks, "input_range", 65536) \
    X(fuzzy_float_eq, "input_range", 65536) \
    X(rsqrt_exact, "input_range", 65536) \
    X(rsqrt_appr, "input_range", 65536) \
    X(rsqrt_appr_nr, "input_range", 65536) \
    X(rsqrt_nr, "input_range", 65536) \
    X(boyer_moore_search, "bytes", 65536) \
    X(KMPSearch, "bytes", 65536) \
    X(computeLPSArray, "bytes", 65536) \
    X(preprocess_bad_char, "batch_parameter", 65536) \
    X(findCycle, "elements", 65536) \
    X(xor_fold, "elements", 65536) \
    X(xxtea_encrypt, "elements", 65536) \
    X(xxtea_decrypt, "elements", 65536) \
    X(floydWarshall, "batch_parameter", 65536) \
    X(bresenham_line, "batch_parameter", 65536) \
    X(bresenham_circle, "batch_parameter", 65536)
#define DECLARE(name, axis, limit) double example_##name(size_t, uint64_t, uint64_t);
EXAMPLE_ALGORITHMS(DECLARE)
#undef DECLARE
#endif
