#include <memory.h>
#include <stdio.h>

unsigned char* unwrap(void (*fn)()) {
    static unsigned char buffer[sizeof(fn)];
    memcpy(buffer, fn, sizeof(fn));
    return buffer;
}

void bar() {};

int main() {
    volatile unsigned char* icall_w = unwrap(bar);

    printf("code:");
    for (int i = 0; i < sizeof(icall_w); i++) {
        printf("%02x ", (unsigned char)icall_w[i]);
    }
    printf("\n");
    return 0;
}
