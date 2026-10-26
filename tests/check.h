#ifndef TEST_CHECK_H
#define TEST_CHECK_H

#include <stdio.h>
#include <stdlib.h>

/* Unlike assert(), checks remain enabled with -DNDEBUG. */
#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: %s failed\n", __FILE__, __LINE__, #condition); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#endif
