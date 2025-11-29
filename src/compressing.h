#ifndef COMPRESSING_H

#define COMPRESSING_H

void rle_encode(const char* input, char* output);

char* rle_decode(const char* input);

#endif    // !COMPRESSING_H
