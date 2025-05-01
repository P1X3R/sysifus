#include "lutgen.h"
#include "bitboard.h"
#include <stdint.h>
#include <stdio.h>

#define LUTS_FILENAME "luts.h"

uint64_t generateJumpingAttack(const Coordinate offsets[JUMPING_OFFSETS],
                               const int8_t square) {
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

  return attack;
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

uint64_t generateSlidingAttack(const Coordinate directions[SLIDING_DIRECTIONS],
                               const int8_t square) {
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

  return attack & generateOccupancyMask(coord);
}

static void write_header(FILE *fptr) {
  (void)fprintf(fptr, "// This file stores generated LUTs. DO NOT MODIFY!\n\n");
  (void)fprintf(fptr, "#pragma once\n\n");
  (void)fprintf(fptr, "#include \"bitboard.h\"\n\n");
}

static void write_jumping_attack_map(FILE *fptr, const char *name,
                                     const Coordinate *offsets,
                                     int num_offsets) {
  (void)fprintf(fptr, "static const uint64_t %s[BOARD_AREA] = {", name);
  for (int8_t square = 0; square < BOARD_AREA; square++) {
    (void)fprintf(fptr, "0x%lx, ", generateJumpingAttack(offsets, square));
  }
  (void)fprintf(fptr, "};\n");
}

static void write_sliding_relevant_mask(FILE *fptr, const char *name,
                                        const Coordinate *directions,
                                        int num_directions) {
  (void)fprintf(fptr, "static const uint64_t %s[BOARD_AREA] = {", name);
  for (int8_t square = 0; square < BOARD_AREA; square++) {
    (void)fprintf(fptr, "0x%lx, ", generateSlidingAttack(directions, square));
  }
  (void)fprintf(fptr, "};\n");
}

void bake(void) {
  FILE *fptr = fopen(LUTS_FILENAME, "w");
  if (!fptr) {
    perror("Error opening LUTs file");
    return;
  }

  write_header(fptr);

  write_jumping_attack_map(fptr, "KNIGHT_ATTACK_MAP", KNIGHT_OFFSETS,
                           JUMPING_OFFSETS);
  write_jumping_attack_map(fptr, "KING_ATTACK_MAP", KING_OFFSETS,
                           JUMPING_OFFSETS);
  write_sliding_relevant_mask(fptr, "BISHOP_RELEVANT_MASK", BISHOP_DIRECTIONS,
                              SLIDING_DIRECTIONS);
  write_sliding_relevant_mask(fptr, "ROOK_RELEVANT_MASK", ROOK_DIRECTIONS,
                              SLIDING_DIRECTIONS);

  (void)fclose(fptr);
}
