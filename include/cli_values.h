/** @file cli_values.h
 * @brief Strict decimal conversions shared by CLI commands and examples.
 */
#ifndef CLI_VALUES_H
#define CLI_VALUES_H
#include <stdint.h>

/** Return 1 for a decimal integer in [0, max], otherwise 0; output is unchanged on error. */
int cli_u64(const char* text, uint64_t max, uint64_t* value);
/** Return 1 for a signed decimal int32, otherwise 0. */
int cli_i32(const char* text, int32_t* value);
/** Return 1 for a finite decimal number in [min, max], otherwise 0. */
int cli_number(const char* text, double min, double max, double* value);
#endif
