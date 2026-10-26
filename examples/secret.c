#include <stdio.h>
#include <string.h>

double m[] = { 7709179928849219.0, 771 };

int main(void) {
    /* Preserve the historical floating-point trick without recursive main(),
       an unbounded string read, or interpreting the result as a format string. */
    for (unsigned i = 0; i < 771; i++) {
        m[0] *= 2;
    }
    char text[sizeof(double) + 1];
    memcpy(text, m, sizeof(double));
    text[sizeof(double)] = '\0';
    puts(text);
    return 0;
}
