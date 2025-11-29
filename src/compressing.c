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
    int input_len = strlen(input);
    char* output = malloc((input_len * 10) + 1);
    if (!output) {
        return NULL;
    }

    int output_index = 0;
    int i = 0;

    while (i < input_len) {
        if (input[i] >= '0' && input[i] <= '9') {
            int count = 0;
            while (i < input_len && input[i] >= '0' && input[i] <= '9') {
                count = count * 10 + (input[i] - '0');
                i++;
            }

            if (i < input_len) {
                char symbol = input[i];
                for (int j = 0; j < count && output_index < input_len * 10; j++) {
                    output[output_index++] = symbol;
                }
                i++;
            }
        } else {
            if (output_index < input_len * 10) {
                output[output_index++] = input[i++];
            } else {
                break;
            }
        }
    }
    output[output_index] = '\0';
    return output;
}
