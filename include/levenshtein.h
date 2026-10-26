#ifndef LEVENSHTEIN_H
#define LEVENSHTEIN_H

/** @brief Edit distance, or -1 on allocation failure or excessive input length. */
int levenshtein(const char* s1, const char* s2);

#endif    // LEVENSHTEIN_H
