#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void computeLPSArray(char* pattern, int m, int* lps) {
    int len = 0;
    lps[0] = 0;
    int i = 1;

    while (i < m) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
}

void KMPSearch(char* pattern, char* text) {
    int m = strlen(pattern);
    int n = strlen(text);

    int* lps = (int*)malloc(m * sizeof(int));
    computeLPSArray(pattern, m, lps);

    int i = 0;
    int j = 0;

    while (i < n) {
        if (pattern[j] == text[i]) {
            j++;
            i++;
        }

        if (j == m) {
            printf("Pattern found at index %d\n", i - j);
            j = lps[j - 1];
        } else if (i < n && pattern[j] != text[i]) {
            if (j != 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }

    free(lps);
}

int main() {
    char text[] = "ABAAABCDABCABCDABCDABDE";
    char pattern[] = "ABCDABD";
    KMPSearch(pattern, text);
    return 0;
}
