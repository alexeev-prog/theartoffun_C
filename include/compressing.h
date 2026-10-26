#ifndef COMPRESSING_H

#define COMPRESSING_H

/** @brief Encode text without digits. Output needs strlen(input) * 3 + 1 bytes. */
void rle_encode(const char* input, char* output);

/** @brief Decode RLE. Caller frees result; NULL on malformed input or allocation failure. */
char* rle_decode(const char* input);

#endif    // !COMPRESSING_H
