#include "bitboard.h"
#include "sysifus.h"
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
  for (int16_t variant = 0; variant < BISHOP_POSSIBLE_VARIANTS; variant++){
    printBitboard(ROOK_ATTACK_MAP[0][variant]);
  }

  return 0;
}
