#include "bitboard.h"
#include "luts.h"
#include <stdint.h>
#include <stdio.h>

void printBitboard(uint64_t bitboard) {
  printf("  A B C D E F G H\n");
  for (int8_t rank = BOARD_LENGTH - 1; rank >= 0; rank--) {
    printf("%d ", rank + 1);
    for (int8_t file = 0; file < BOARD_LENGTH; file++) {
      printf("%c ", isSet((Coordinate){rank, file}, bitboard) ? '#' : '.');
    }
    printf("\n");
  }
}

int main(void) {
  for (int square = 0; square < BOARD_AREA; square++) {
    printBitboard(KING_ATTACK_MAP[square]);
  }

  return 0;
}
