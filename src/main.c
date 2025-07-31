#ifdef _WIN32
#include <windows.h>
#endif
#include "fib_algos.h"
#include "cmdparser.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif
    int help_flag = 0;
    char *fib_value = NULL;
    char *basic_value = NULL;
    char *fib_interp_value = NULL;
    char *fib_cache_value = NULL;
    char *fib_golden_value = NULL;

    struct CommandOption options[] = {
            {.help = "Show help information",
             .long_name = "help",
             .short_name = 'h',
             .has_arg = 0,
             .default_value = NULL,
             .handler = &help_flag},
            {.help = "Convert miles to km using basic Fibonacci",
             .long_name = "fib",
             .short_name = 'f',
             .has_arg = 1,
             .default_value = NULL,
             .handler = &fib_value},
            {.help = "Convert miles to km using standard formula",
             .long_name = "basic",
             .short_name = 'b',
             .has_arg = 1,
             .default_value = NULL,
             .handler = &basic_value},
            {.help = "Convert using Fibonacci interpolation",
             .long_name = "fib-interp",
             .short_name = 'i',
             .has_arg = 1,
             .default_value = NULL,
             .handler = &fib_interp_value},
            {.help = "Convert using cached Fibonacci",
             .long_name = "fib-cache",
             .short_name = 'c',
             .has_arg = 1,
             .default_value = NULL,
             .handler = &fib_cache_value},
            {.help = "Convert using golden ratio",
             .long_name = "fib-golden",
             .short_name = 'g',
             .has_arg = 1,
             .default_value = NULL,
             .handler = &fib_golden_value}};

    struct CLIMetadata meta = {
            .prog_name = argv[0],
            .description = "TheArtOfFun-C",
            .usage_args = "[commands]",
            .options = options,
            .options_count = sizeof(options) / sizeof(options[0])};

    int pos_index = parse_options(argc, argv, meta.options, meta.options_count);
    if (pos_index < 0) {
        return EXIT_FAILURE;
    }

    if (help_flag) {
        print_help(&meta);
        return EXIT_SUCCESS;
    }

    // Check for conflicts
    int fib_methods_count = 0;
    if (fib_value)
        fib_methods_count++;
    if (fib_interp_value)
        fib_methods_count++;
    if (fib_cache_value)
        fib_methods_count++;
    if (fib_golden_value)
        fib_methods_count++;

    if (fib_methods_count > 1) {
        fprintf(stderr, "Error: Use only one Fibonacci conversion method\n");
        return EXIT_FAILURE;
    }
    if (basic_value && fib_methods_count > 0) {
        fprintf(stderr, "Error: Cannot combine basic and Fibonacci conversions\n");
        return EXIT_FAILURE;
    }

    // Handle --fib
    if (fib_value) {
        char *endptr;
        double miles = strtod(fib_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", fib_value);
            return EXIT_FAILURE;
        }

        uint64_t km = fibonacci(miles + 1.0);
        printf("%.2f miles ≈ %" PRIu64 " km (Fibonacci)\n", miles, km);
        return EXIT_SUCCESS;
    }

    // Handle --fib-interp
    if (fib_interp_value) {
        char *endptr;
        float miles = strtof(fib_interp_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", fib_interp_value);
            return EXIT_FAILURE;
        }

        float km = fib_interpolate(miles);
        printf("%.2f miles ≈ %.2f km (Fibonacci interpolation)\n", miles, km);
        return EXIT_SUCCESS;
    }

    // Handle --fib-cache
    if (fib_cache_value) {
        char *endptr;
        float miles = strtof(fib_cache_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", fib_cache_value);
            return EXIT_FAILURE;
        }

        float km = fib_cache_convert(miles);
        printf("%.2f miles ≈ %.2f km (Cached Fibonacci)\n", miles, km);
        return EXIT_SUCCESS;
    }

    // Handle --fib-golden
    if (fib_golden_value) {
        char *endptr;
        float miles = strtof(fib_golden_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", fib_golden_value);
            return EXIT_FAILURE;
        }

        float km = fib_golden_ratio(miles);
        printf("%.2f miles ≈ %.2f km (Golden Ratio)\n", miles, km);
        return EXIT_SUCCESS;
    }

    // Handle --basic
    if (basic_value) {
        char *endptr;
        float miles = strtod(basic_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", basic_value);
            return EXIT_FAILURE;
        }

        float km = basic_miles2km(miles);
        printf("%f miles = %f km\n", miles, km);
        return EXIT_SUCCESS;
    }

    // Handle positional arguments
    if (pos_index < argc) {
        for (int i = pos_index; i < argc; i++) {
            char *endptr;
            double miles = strtod(argv[i], &endptr);

            if (*endptr != '\0' || miles < 0) {
                fprintf(stderr, "Error: Invalid distance value '%s'. Skipping.\n",
                                argv[i]);
                continue;
            }

            float km = basic_miles2km(miles);
            printf("%.2f miles = %.2f km\n", miles, km);
        }
        return EXIT_SUCCESS;
    }

    // No arguments provided
    print_help(&meta);
    return EXIT_SUCCESS;
}
