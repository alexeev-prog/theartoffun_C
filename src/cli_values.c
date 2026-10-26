#include "cli_values.h"

#include <errno.h>
#include <math.h>
#include <stdlib.h>

int cli_u64(const char* text, uint64_t max, uint64_t* value) {
    if (!text || !*text || !value) {
        return 0;
    }
    for (const char* p = text; *p; p++) {
        if (*p < '0' || *p > '9') {
            return 0;
        }
    }
    errno = 0;
    char* end;
    unsigned long long result = strtoull(text, &end, 10);
    if (errno || *end || result > max) {
        return 0;
    }
    *value = (uint64_t)result;
    return 1;
}

int cli_number(const char* text, double min, double max, double* value) {
    if (!text || !*text || !value) {
        return 0;
    }
    /* Exclude whitespace, hexadecimal floats, NaN and infinity explicitly. */
    for (const char* p = text; *p; p++) {
        if (!((*p >= '0' && *p <= '9') || *p == '+' || *p == '-' || *p == '.' || *p == 'e' || *p == 'E')) {
            return 0;
        }
    }
    errno = 0;
    char* end;
    double result = strtod(text, &end);
    if (errno || end == text || *end || !isfinite(result) || result < min || result > max) {
        return 0;
    }
    *value = result;
    return 1;
}

int cli_i32(const char* text, int32_t* value) {
    if (!text || !value) {
        return 0;
    }
    int negative = *text == '-';
    if (negative || *text == '+') {
        text++;
    }
    uint64_t magnitude;
    if (!cli_u64(text, negative ? UINT64_C(2147483648) : INT32_MAX, &magnitude)) {
        return 0;
    }
    *value = negative ? (int32_t)(-(int64_t)magnitude) : (int32_t)magnitude;
    return 1;
}
