#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdint.h>

// Basic implementations
uint64_t fibonacci(int num);
float basic_miles2km(float miles);

/**
 * @brief Convert miles to km using Fibonacci interpolation
 *
 * @param miles Distance in miles (0-100000)
 * @return float Approximate distance in kilometers
 */
float fib_interpolate(float miles);

/**
 * @brief Convert miles to km using Fibonacci with cache
 *
 * @param miles Distance in miles (0-100000)
 * @return float Approximate distance in kilometers
 */
float fib_cache_convert(float miles);

/**
 * @brief Convert miles to km using golden ratio
 *
 * @param miles Distance in miles (0-100000)
 * @return float Approximate distance in kilometers
 */
float fib_golden_ratio(float miles);

#endif
