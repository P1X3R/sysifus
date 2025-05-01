#include "lutgen.h"
#include "bitboard.h"
#include <stdint.h>

void generateJumpingAttackMap(const Coordinate offsets[JUMPING_OFFSETS],
                              uint64_t lut[BOARD_LENGTH]) {
  for (uint8_t square = 0; square < BOARD_AREA; square++) {
    Coordinate coord = {
        .rank = (int8_t)(square / BOARD_LENGTH),
        .file = (int8_t)(square % BOARD_LENGTH),
    };
    uint64_t attack = 0;

    for (int8_t offsetIndex = 0; offsetIndex < JUMPING_OFFSETS; offsetIndex++) {
      const Coordinate attackedCoord = {
          (int8_t)(coord.rank + offsets[offsetIndex].rank),
          (int8_t)(coord.file + offsets[offsetIndex].file),
      };

      if (isCoordValid(attackedCoord)) {
        attack |= 1ULL << coordToSquare(attackedCoord);
      }
    }

    lut[square] = attack;
  }
}

static uint64_t generateOccupancyMask(const Coordinate coord) {
  // 01111110
  // 11111111
  // 11111111
  // 11111111
  // 11111111
  // 11111111
  // 11111111
  // 01111110
  const uint64_t baseMask = 0x7EFFFFFFFFFFFF7E;

  uint64_t mask = baseMask;
  if (coord.rank != 0) {
    // 11111111
    // 11111111
    // 11111111
    // 11111111
    // 11111111
    // 11111111
    // 11111111
    // 00000000
    const uint64_t rank1 = 0xFFFFFFFFFFFFFF00;
    mask &= rank1;
  }
  if (coord.rank != BOARD_LENGTH - 1) {
    // 00000000
    // 11111111
    // 11111111
    // 11111111
    // 11111111
    // 11111111
    // 11111111
    // 11111111
    const uint64_t rank8 = 0xFFFFFFFFFFFFFF;
    mask &= rank8;
  }
  if (coord.file != 0) {
    // 11111110
    // 11111110
    // 11111110
    // 11111110
    // 11111110
    // 11111110
    // 11111110
    // 11111110
    const uint64_t file8 = 0xFEFEFEFEFEFEFEFE;
    mask &= file8;
  }
  if (coord.file != BOARD_LENGTH - 1) {

    // 01111111
    // 01111111
    // 01111111
    // 01111111
    // 01111111
    // 01111111
    // 01111111
    // 01111111
    const uint64_t file1 = 0x7F7F7F7F7F7F7F7F;
    mask &= file1;
  }

  return mask;
}

void generateSlidingAttackMap(const Coordinate directions[SLIDING_DIRECTIONS],
                              uint64_t lut[BOARD_LENGTH]) {
  for (uint8_t square = 0; square < BOARD_AREA; square++) {
    Coordinate coord = {
        .rank = (int8_t)(square / BOARD_LENGTH),
        .file = (int8_t)(square % BOARD_LENGTH),
    };
    uint64_t attack = 0;

    for (int8_t directionIndex = 0; directionIndex < SLIDING_DIRECTIONS;
         directionIndex++) {
      uint64_t ray = 0;

      for (Coordinate attackedCoord =
               {(int8_t)(coord.rank + directions[directionIndex].rank),
                (int8_t)(coord.file + directions[directionIndex].file)};
           isCoordValid(attackedCoord);
           attackedCoord = (Coordinate){
               (int8_t)(attackedCoord.rank + directions[directionIndex].rank),
               (int8_t)(attackedCoord.file + directions[directionIndex].file),
           }) {
        ray |= 1ULL << coordToSquare(attackedCoord);
      }

      attack |= ray;
    }

    lut[square] = attack & generateOccupancyMask(coord);
  }
}
