/** @file pi_leibniz.c @brief A finite, configurable Leibniz-series experiment. */
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#include "algos.h"
#include "cli_values.h"

int main(int argc, char** argv) {
    uint64_t iterations = 100000;
    if (argc > 2 || (argc == 2 && (!cli_u64(argv[1], 10000000, &iterations) || !iterations))) {
        fprintf(stderr, "Usage: %s [ITERATIONS] (1..10000000)\n", argv[0]);
        return 1;
    }
    clock_t start = clock();
    double pi = calculate_pi_leibniz((long long)iterations);
    printf(
        "pi = %.15f\nIterations: %" PRIu64 "\nCPU time: %.3f seconds\n",
        pi,
        iterations,
        (double)(clock() - start) / CLOCKS_PER_SEC);
    return 0;
}
