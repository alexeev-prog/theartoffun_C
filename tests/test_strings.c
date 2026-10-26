#include "compressing.h"
#include "levenshtein.h"
#include "check.h"

#include <string.h>

int main(void) {
    CHECK(levenshtein("kitten", "sitting") == 3);
    CHECK(levenshtein("", "abc") == 3);
    CHECK(levenshtein("same", "same") == 0);
    CHECK(levenshtein("abcd", "xy") == levenshtein("xy", "abcd"));
    const char* samples[] = {"", "a", "aaabbc", "abcdef", "zzzzzzzzzzzzzzzzzzzz"};
    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); i++) {
        char encoded[128];
        rle_encode(samples[i], encoded);
        char* decoded = rle_decode(encoded);
        CHECK(decoded != NULL);
        CHECK(strcmp(decoded, samples[i]) == 0);
        free(decoded);
    }
    char* long_run = rle_decode("1000a");
    CHECK(long_run != NULL && strlen(long_run) == 1000);
    CHECK(strspn(long_run, "a") == 1000);
    free(long_run);
    CHECK(rle_decode("12") == NULL);
    CHECK(rle_decode("0a") == NULL);
    CHECK(rle_decode("999999999999999999999999999999a") == NULL);
    puts("String/compression tests passed");
    return 0;
}
