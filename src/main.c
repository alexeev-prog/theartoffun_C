#ifdef _WIN32
#    include <windows.h>
#endif
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "algos.h"
#include "cmdparser.h"
#include "fib_algos.h"
#include "pow_algos.h"

uint64_t get_seed() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    int help_flag = 0;
    char* fib_value = NULL;
    char* basic_value = NULL;
    char* fib_interp_value = NULL;
    char* fib_cache_value = NULL;
    char* fib_golden_value = NULL;
    char* binary_power = NULL;
    char* xorshift_flag = 0;
    char* xorshift_double_flag = 0;
    char* q_rsqrt_quake = NULL;
    char* fib_golden_binary_value = NULL;
    char* lehmer64_value_flag = 0;
    char* xoshiro256pp_flag = 0;

    char* exponent = NULL;

    struct CommandOption options[] = {
        {.help = "Show help information",
         .long_name = "help",
         .short_name = 'h',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &help_flag},
        {.help = "Convert miles to km miles to km using basic Fibonacci",
         .long_name = "fib",
         .short_name = 'f',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fib_value},
        {.help = "Convert miles to km miles to km using standard formula",
         .long_name = "miles-to-km",
         .short_name = 'm',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &basic_value},
        {.help = "Convert miles to km using Fibonacci interpolation",
         .long_name = "fib-interp",
         .short_name = 'i',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fib_interp_value},
        {.help = "Convert miles to km using cached Fibonacci",
         .long_name = "fib-cache",
         .short_name = 'c',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fib_cache_value},
        {.help = "Convert miles to km using golden ratio",
         .long_name = "fib-golden",
         .short_name = 'g',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fib_golden_value},
        {.help = "Set exponent for pow-algos",
         .long_name = "exponent",
         .short_name = 'e',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &exponent},
        {.help = "Power the number by binary power algorithm",
         .long_name = "binary-power",
         .short_name = 'p',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &binary_power},
        {.help = "Generate pseudo random numbers by xorshift64",
         .long_name = "xorshift-random",
         .short_name = 'x',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &xorshift_flag},
        {.help = "Generate pseudo random float numbers by xorshift64",
         .long_name = "xorshift-double-random",
         .short_name = 'd',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &xorshift_double_flag},
        {.help = "Q_rsqrt from Quake III Arena",
         .long_name = "q-rsqrt-quake",
         .short_name = 'q',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &q_rsqrt_quake},
        {.help = "Convert miles to km using golden ratio with binary pow",
         .long_name = "fib-golden-binry",
         .short_name = 'b',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fib_golden_binary_value},
        {.help = "Generate pseudo random numbers by lehmer64",
         .long_name = "lehmer-random",
         .short_name = 'l',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &lehmer64_value_flag},
        {.help = "Generate pseudo random numbers by xoshiro256pp",
         .long_name = "xorshiro256pp-random",
         .short_name = 'o',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &xoshiro256pp_flag},
    };

    struct CLIMetadata meta = {.prog_name = argv[0],
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
    if (fib_value) {
        fib_methods_count++;
    }
    if (fib_interp_value) {
        fib_methods_count++;
    }
    if (fib_cache_value) {
        fib_methods_count++;
    }
    if (fib_golden_value) {
        fib_methods_count++;
    }

    if (fib_methods_count > 1) {
        fprintf(stderr, "Error: Use only one Fibonacci conversion method\n");
        return EXIT_FAILURE;
    }
    if (basic_value && fib_methods_count > 0) {
        fprintf(stderr, "Error: Cannot combine basic and Fibonacci conversions\n");
        return EXIT_FAILURE;
    }

    // Handle --binary_power
    if (binary_power && exponent) {
        char* endptr;
        double number = strtod(binary_power, &endptr);
        double e = strtod(exponent, &endptr);
        if (*endptr != '\0' || number < 0) {
            fprintf(stderr, "Error: Invalid base value '%s'\n", binary_power);
            return EXIT_FAILURE;
        }

        if (*endptr != '\0' || e < 0) {
            fprintf(stderr, "Error: Invalid exponent value '%s'\n", binary_power);
            return EXIT_FAILURE;
        }

        float powered_number = binary_pow(number, e);
        printf("%.2f ** %.2f = %.2f\n", number, e, powered_number);
        return EXIT_SUCCESS;
    }

    if (xorshift_flag) {
        uint64_t seed = get_seed();
        uint64_t num = xorshift64(&seed);
        uint64_t min_num = 10;
        uint64_t max_num = 100;
        uint64_t ranged_num = rand_range(&seed, min_num, max_num);
        printf("xorshift64 random number: %lu\n", num);
        printf("xorshift64 random num from 10 to 100: %lu\n", ranged_num);
        return EXIT_SUCCESS;
    }

    if (xorshift_double_flag) {
        uint64_t seed = get_seed();
        double num = rand_double(&seed);
        printf("xorshift64 double random number: %f\n", num);
        return EXIT_SUCCESS;
    }

    if (lehmer64_value_flag) {
        uint64_t number = lehmer64();

        printf("lehmer64 random number: %lu", number);
        return EXIT_SUCCESS;
    }

    if (xoshiro256pp_flag) {
        uint64_t seed = get_seed();
        xoshiro256pp_state state;
        xoshiro256pp_init(&state, seed);

        printf("xorshiro256pp random number: %lu\n", xoshiro256pp_next(&state));
        return EXIT_SUCCESS;
    }

    if (q_rsqrt_quake) {
        char* endptr;
        float number = strtof(q_rsqrt_quake, &endptr);

        if (endptr == q_rsqrt_quake || *endptr != '\0') {
            fprintf(stderr, "Error: Invalid number format for Q_rsqrt\n");
            return EXIT_FAILURE;
        }

        if (number <= 0.0f) {
            fprintf(stderr, "Error: Q_rsqrt requires positive number\n");
            return EXIT_FAILURE;
        }

        float result = Q_rsqrt(number);
        printf("Q_rsqrt(%.2f) = %f\n", number, result);
        return EXIT_SUCCESS;
    }

    // Handle --fib
    if (fib_value) {
        char* endptr;
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
        char* endptr;
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
        char* endptr;
        float miles = strtof(fib_cache_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", fib_cache_value);
            return EXIT_FAILURE;
        }

        float km = fib_cache_convert(miles);
        printf("%.2f miles ≈ %.2f km (Cached Fibonacci)\n", miles, km);
        return EXIT_SUCCESS;
    }

    // Handle --fib-golden-binary
    if (fib_golden_binary_value) {
        char* endptr;
        float miles = strtof(fib_golden_binary_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", fib_golden_binary_value);
            return EXIT_FAILURE;
        }

        float km = fib_golden_ratio_binary(miles);
        printf("%.2f miles ≈ %.2f km (Golden Ratio with Binary Pow)\n", miles, km);
        return EXIT_SUCCESS;
    }

    // Handle --fib-golden
    if (fib_golden_value) {
        char* endptr;
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
        char* endptr;
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
            char* endptr;
            double miles = strtod(argv[i], &endptr);

            if (*endptr != '\0' || miles < 0) {
                fprintf(stderr, "Error: Invalid distance value '%s'. Skipping.\n", argv[i]);
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
