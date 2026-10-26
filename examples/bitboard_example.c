#include <stdint.h>
#include <stdio.h>

typedef uint64_t Bitboard;

Bitboard knights_attacks(Bitboard knights) {
    Bitboard l1 = (knights >> 1) & 0x7F7F7F7F7F7F7F7F;
    Bitboard l2 = (knights >> 2) & 0x3F3F3F3F3F3F3F3F;
    Bitboard r1 = (knights << 1) & 0xFEFEFEFEFEFEFEFE;
    Bitboard r2 = (knights << 2) & 0xFCFCFCFCFCFCFCFC;
    return (l1 | r1) << 16 | (l1 | r1) >> 16 | (l2 | r2) << 8 | (l2 | r2) >> 8;
}

void print_bitboard(Bitboard bb) {
    for (int row = 7; row >= 0; row--) {
        for (int col = 0; col < 8; col++) {
            int square = row * 8 + col;
            printf("%c ", (bb >> square) & 1 ? '1' : '.');
        }
        printf("\n");
    }
    printf("\n");
}

int main() {
    Bitboard knight = 1ULL << 36;    // Knight on E4

    printf("Позиция коня:\n");
    print_bitboard(knight);

    printf("Возможные ходы коня:\n");
    Bitboard attacks = knights_attacks(knight);
    print_bitboard(attacks);

    return 0;
}
