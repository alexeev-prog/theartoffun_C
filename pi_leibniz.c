#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

double calculate_pi_leibniz(long long iterations) {
    double pi = 1.0;
    long long i;
    int sign = -1;

#pragma omp parallel for reduction(+ : pi) private(sign)
    for (i = 1; i < iterations; i++) {
        sign = (i % 2 == 0) ? 1 : -1;
        pi += sign / (2.0 * i + 1.0);
    }

    return pi * 4;
}

int main(int argc, char* argv[]) {
    long long iterations = 1000000000;
    if (argc > 1) {
        iterations = atoll(argv[1]);
    }

    double start_time = omp_get_wtime();
    double pi = calculate_pi_leibniz(iterations);
    double end_time = omp_get_wtime();

    printf("π = %.15f\n", pi);
    printf("Iters: %lld\n", iterations);
    printf("time: %.3f seconds\n", end_time - start_time);

    return 0;
}
