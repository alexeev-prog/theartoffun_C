#ifndef COMPRESSING_H

#define COMPRESSING_H

#include <stddef.h>

/** @brief Encode text without digits. Output needs strlen(input) * 3 + 1 bytes. */
void rle_encode(const char* input, char* output);

/** @brief Decode RLE. Caller frees result; NULL on malformed input or allocation failure. */
char* rle_decode(const char* input);

/** @brief Decode at most max_output bytes (excluding NUL); NULL on error/limit. */
char* rle_decode_limit(const char* input, size_t max_output);

#endif    // !COMPRESSING_H
