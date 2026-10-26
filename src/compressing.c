#include "compressing.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void rle_encode(const char* input, char* output) {
    int input_len = strlen(input);
    int output_index = 0;
    int count;
    char current_char;

    for (int i = 0; i < input_len; i++) {
        current_char = input[i];
        count = 1;

        while (i + 1 < input_len && input[i] == input[i + 1]) {
            count++;
            i++;
        }

        if (count > 1) {
            int written = snprintf(
                output + output_index, (input_len * 3 + 1) - output_index, "%d%c", count, current_char);
            if (written < 0) {
                break;
            }
            output_index += written;
        } else {
            if (output_index < input_len * 3) {
                output[output_index++] = current_char;
            } else {
                break;
            }
        }
    }
    output[output_index] = '\0';
}

char* rle_decode(const char* input) {
    return rle_decode_limit(input, SIZE_MAX - 1);
}

char* rle_decode_limit(const char* input, size_t max_output) {
    if (!input) {
        return NULL;
    }
    size_t length = 0;
    const char* p = input;
    while (*p) {
        size_t count = 1;
        if (*p >= '0' && *p <= '9') {
            count = 0;
            do {
                unsigned digit = (unsigned)(*p++ - '0');
                if (count > (SIZE_MAX - digit) / 10) {
                    return NULL;
                }
                count = count * 10 + digit;
            } while (*p >= '0' && *p <= '9');
            if (!*p || count == 0) {
                return NULL;
            }
        }
        if (count > SIZE_MAX - 1 - length) {
            return NULL;
        }
        length += count;
        if (length > max_output) {
            return NULL;
        }
        p++;
    }

    char* output = malloc(length + 1);
    if (!output) {
        return NULL;
    }
    size_t offset = 0;
    p = input;
    while (*p) {
        size_t count = 1;
        if (*p >= '0' && *p <= '9') {
            count = 0;
            while (*p >= '0' && *p <= '9') {
                count = count * 10 + (unsigned)(*p++ - '0');
            }
        }
        memset(output + offset, *p++, count);
        offset += count;
    }
    output[offset] = '\0';
    return output;
}
