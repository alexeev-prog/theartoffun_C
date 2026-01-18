#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET_SIZE 256

void preprocess_bad_char(char* pattern, int pattern_len, int bad_char[]) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        bad_char[i] = pattern_len;
    }
    for (int i = 0; i < pattern_len - 1; i++) {
        bad_char[(unsigned char)pattern[i]] = pattern_len - 1 - i;
    }
}

int boyer_moore_search(char* text, char* pattern) {
    int text_len = strlen(text);
    int pattern_len = strlen(pattern);

    if (pattern_len == 0) {
        return 0;
    }

    int bad_char[ALPHABET_SIZE];
    preprocess_bad_char(pattern, pattern_len, bad_char);

    int shift = 0;
    while (shift <= text_len - pattern_len) {
        int j = pattern_len - 1;

        while (j >= 0 && pattern[j] == text[shift + j]) {
            j--;
        }

        if (j < 0) {
            return shift;
        }

        int bad_char_shift = bad_char[(unsigned char)text[shift + j]] - (pattern_len - 1 - j);
        shift += bad_char_shift > 0 ? bad_char_shift : 1;
    }

    return -1;
}

int main() {
    char text[] = "ABAAABCDABCABCDABCDABDE";
    char pattern[] = "ABCDABD";

    int result = boyer_moore_search(text, pattern);

    if (result != -1) {
        printf("Pattern found at index: %d\n", result);
    } else {
        printf("Pattern not found\n");
    }

    return 0;
}
