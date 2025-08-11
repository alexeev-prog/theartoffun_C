#ifndef FIB_ALGORITHMS_H
#define FIB_ALGORITHMS_H

#include <stdint.h>

// Basic implementations
uint64_t fibonacci(int num);

/**
 * @brief Basic convertion miles to km using constant
 *
 * @param miles
 * @return float
 **/
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

/**
 * @brief Convert miles to km using golden ratio and binary power
 *
 * @param miles Distance in miles
 * @return float Approximate distance in kilometres
 **/
float fib_golden_ratio_binary(float miles);

#endif
