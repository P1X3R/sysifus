#include "sysifus.h"
#include "bitboard.h"
#include "luts.h"
#include <assert.h>
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

uint64_t generatePawnPushes(const Coordinate coord,
                            const uint64_t blockedSquares, const bool isWhite) {
  if (!isCoordValid(coord)) {
    return 0;
  }

  uint64_t pushes = 1ULL << coordToSquare(coord);

  if (isWhite) {
    pushes = (pushes << BOARD_LENGTH) & ~blockedSquares;

    const int8_t starterRank = 1;
    if (coord.rank == starterRank) {
      pushes |= (pushes << BOARD_LENGTH) & ~blockedSquares;
    }
  } else {
    pushes = (pushes >> BOARD_LENGTH) & ~blockedSquares;

    const int8_t starterRank = 6;
    if (coord.rank == starterRank) {
      pushes |= (pushes >> BOARD_LENGTH) & ~blockedSquares;
    }
  }

  return pushes;
}

uint64_t generatePawnCaptures(const Coordinate coord, const uint64_t enemy,
                              const bool isWhite) {
  if (!isCoordValid(coord)) {
    return 0;
  }

  const int8_t pawnDirection = isWhite ? 1 : -1;
  const Coordinate leftKill = {
      .rank = (int8_t)(coord.rank + pawnDirection),
      .file = (int8_t)(coord.file - 1),
  };
  const Coordinate rightKill = {
      .rank = (int8_t)(leftKill.rank),
      .file = (int8_t)(coord.file + 1),
  };
  uint64_t captures = 0;

  if (isCoordValid(leftKill)) {
    captures |= 1ULL << coordToSquare(leftKill);
  }
  if (isCoordValid(rightKill)) {
    captures |= 1ULL << coordToSquare(rightKill);
  }

  return captures & enemy;
}

static uint64_t generateJumpingAttack(const Coordinate offsets[JUMPING_OFFSETS],
                                      const int8_t square) {
#ifndef NDEBUG
  assert(offsets != NULL);
#endif /* ifndef  NDEBUG */

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

static uint64_t
generateSlidingAttack(const uint64_t occupancy,
                      const Coordinate directions[SLIDING_DIRECTIONS],
                      const int8_t square) {
#ifndef NDEBUG
  assert(directions != NULL);
#endif /* ifndef  NDEBUG */

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

      if (isSet(attackedCoord, occupancy)) {
        break;
      }
    }

    attack |= ray;
  }

  return attack;
}

typedef struct {
  uint64_t mask;
} RelevantMask;

static void generateOccupancyVariants(const RelevantMask relevantMask,
                                      const uint16_t possibleVariants,
                                      uint64_t variants[possibleVariants]) {
#ifndef NDEBUG
  assert(variants != NULL);
#endif

  for (uint16_t variantIndex = 0; variantIndex < possibleVariants;
       variantIndex++) {
    // Iterate over bits of the variant index Brian Kernighan's way
    // https://www.geeksforgeeks.org/count-set-bits-in-an-integer/
    uint64_t variantIndexTemp = variantIndex;
    uint64_t relevantMaskTemp = relevantMask.mask;
    uint64_t occupancy = 0;

    while (variantIndexTemp) {
      // This a & -a just isolate the least significant bit of number
      const uint64_t relevantMaskLSB = relevantMaskTemp & -relevantMaskTemp;
      const bool isCurrentBitSet = variantIndexTemp & 1;

      if (isCurrentBitSet) {
        occupancy |= relevantMaskLSB; // Append the relevant  mask's lsb
      }

      // Delete the lsb from the relevant mask
      relevantMaskTemp &= ~relevantMaskLSB;
      variantIndexTemp >>= 1; // Move one bit forward
    }

    variants[variantIndex] = occupancy;
  }
}

static uint16_t getVariantIndex(const uint64_t occupancy,
                                const RelevantMask relevantMask) {
  // Filter only relevant bits and compress them to LSB positions
  const uint64_t occupied = occupancy & relevantMask.mask;

// Use pext instruction if available (Intel/AMD CPUs with BMI2)
#if defined(__BMI2__)
  return _pext_u64(occupied, relevantMask.mask);
#else
  // Fallback: Multiply-and-mask technique for bit compression
  const uint64_t spacedBits = occupied * relevantMask.mask;
  return (spacedBits >>
          (BOARD_AREA - __builtin_popcountll(relevantMask.mask))) &
         ((1 << __builtin_popcountll(relevantMask.mask)) - 1);
#endif
}

static void
generateSlidingRelevantMasksLUT(const Coordinate directions[SLIDING_DIRECTIONS],
                                uint64_t lut[BOARD_AREA]) {
  for (int8_t square = 0; square < BOARD_AREA; square++) {
    lut[square] = generateSlidingAttack(0, directions, square) &
                  generateOccupancyMask((Coordinate){
                      .rank = (int8_t)(square / BOARD_LENGTH),
                      .file = (int8_t)(square % BOARD_LENGTH),
                  });
  }
}

static void writeHeader(FILE *fptr) {
  (void)fprintf(fptr, "// This file stores generated LUTs. DO NOT MODIFY!\n\n");
  (void)fprintf(fptr, "#pragma once\n\n");
  (void)fprintf(fptr, "#include \"bitboard.h\"\n\n");
}

static void writeJumpingAttackMap(FILE *fptr, const char *name,
                                  const Coordinate offsets[JUMPING_OFFSETS]) {
  (void)fprintf(fptr, "static const uint64_t %s[BOARD_AREA] = {", name);
  for (int8_t square = 0; square < BOARD_AREA; square++) {
    (void)fprintf(fptr, (square < BOARD_AREA - 1) ? "0x%016lx, " : "0x%016lx",
                  generateJumpingAttack(offsets, square));
  }
  (void)fprintf(fptr, "};\n");
}

static void writeSlidingRelevantMask(FILE *fptr, const char *name,
                                     const uint64_t lut[BOARD_AREA]) {
  (void)fprintf(fptr, "static const uint64_t %s[BOARD_AREA] = {", name);
  for (int8_t square = 0; square < BOARD_AREA; square++) {
    (void)fprintf(fptr, (square < BOARD_AREA - 1) ? "0x%016lx, " : "0x%016lx",
                  lut[square]);
  }
  (void)fprintf(fptr, "};\n");
}

static void
writeSlidingAttackMap(FILE *fptr, const char *name,
                      const uint64_t relevantMasks[BOARD_AREA],
                      const uint16_t possibleVariants,
                      const Coordinate directions[SLIDING_DIRECTIONS]) {
  (void)fprintf(fptr, "static const uint64_t %s[BOARD_AREA][%d] = {", name,
                possibleVariants);
  for (int8_t square = 0; square < BOARD_AREA; square++) {
    uint64_t occupancyVariants[possibleVariants];
    generateOccupancyVariants((RelevantMask){relevantMasks[square]},
                              possibleVariants, occupancyVariants);

    (void)fprintf(fptr, "{");
    for (uint16_t variantIndex = 0; variantIndex < possibleVariants;
         variantIndex++) {
      (void)fprintf(fptr,
                    (variantIndex < possibleVariants - 1) ? "0x%016lx, "
                                                          : "0x%016lx",
                    generateSlidingAttack(occupancyVariants[variantIndex],
                                          directions, square));
    }
    (void)fprintf(fptr, (square < BOARD_AREA - 1) ? "}, " : "}");
  }
  (void)fprintf(fptr, "};\n");
}

static uint64_t BISHOP_RELEVANT_MASK_TEMP[BOARD_AREA];
static uint64_t ROOK_RELEVANT_MASK_TEMP[BOARD_AREA];

void bake(void) {
  FILE *fptr = fopen("luts.h", "w");
  if (!fptr) {
    perror("Error opening LUTs file");
    return;
  }

  generateSlidingRelevantMasksLUT(BISHOP_DIRECTIONS, BISHOP_RELEVANT_MASK_TEMP);
  generateSlidingRelevantMasksLUT(ROOK_DIRECTIONS, ROOK_RELEVANT_MASK_TEMP);

  writeHeader(fptr);

  writeJumpingAttackMap(fptr, "KNIGHT_ATTACK_MAP", KNIGHT_OFFSETS);
  writeJumpingAttackMap(fptr, "KING_ATTACK_MAP", KING_OFFSETS);
  writeSlidingRelevantMask(fptr, "BISHOP_RELEVANT_MASK",
                           BISHOP_RELEVANT_MASK_TEMP);
  writeSlidingRelevantMask(fptr, "ROOK_RELEVANT_MASK", ROOK_RELEVANT_MASK_TEMP);
  writeSlidingAttackMap(fptr, "BISHOP_ATTACK_MAP", BISHOP_RELEVANT_MASK_TEMP,
                        (uint16_t)BISHOP_POSSIBLE_VARIANTS, BISHOP_DIRECTIONS);
  writeSlidingAttackMap(fptr, "ROOK_ATTACK_MAP", ROOK_RELEVANT_MASK_TEMP,
                        (uint16_t)ROOK_POSSIBLE_VARIANTS, ROOK_DIRECTIONS);

  if (ferror(fptr)) {
    perror("Error writing to LUTs file");
    fclose(fptr);
    return;
  }

  if (fclose(fptr) != 0) {
    perror("Error closing LUTs file");
    return;
  }
}

uint64_t getAttackByOccupancy(const int8_t square,
                              const uint64_t relevantMask[BOARD_AREA],
                              const uint16_t possibleVariants,
                              const uint64_t lut[BOARD_AREA][possibleVariants],
                              const uint64_t friendly, const uint64_t enemy) {
#ifndef NDEBUG
  assert(lut != NULL);
  assert(relevantMask != NULL);
#endif /* ifndef NDEBUG */

  if (square < 0 || square >= BOARD_AREA) {
    return 0;
  }

  const uint16_t variantIndex =
      getVariantIndex(friendly | enemy, (RelevantMask){relevantMask[square]});

#ifndef NDEBUG
  assert(variantIndex < possibleVariants);
#endif /* ifndef NDEBUG */

  return lut[square][variantIndex] & ~friendly;
}

// WARNING: For king pseudo-legal you need to delete the attacked squares, you
// can do it in the following way: kingAttacks & ~attackedSquares.
// WARNING: For the pawn moves, it doesn't calculate the pawn promotions or en
// passant, you have to handle them yourself.
Move getPseudoLegal(const Piece type, const Coordinate coord,
                    const uint64_t friendly, const bool isWhite,
                    const uint64_t enemy) {
  Move move = {0, 0};
  const int8_t square = coordToSquare(coord);
  const uint64_t blocked = friendly | enemy;

  switch (type) {
  case PAWN:
    move.quiet = generatePawnPushes(coord, blocked, isWhite);
    move.kills = generatePawnCaptures(coord, enemy, isWhite);
    break;
  case KNIGHT:
    move.quiet = KNIGHT_ATTACK_MAP[square] & ~blocked;
    move.kills = KNIGHT_ATTACK_MAP[square] & enemy;
    break;
  case BISHOP: {
    const uint64_t attacks = getAttackByOccupancy(
        square, BISHOP_RELEVANT_MASK, BISHOP_POSSIBLE_VARIANTS,
        BISHOP_ATTACK_MAP, friendly, enemy);
    move.quiet = attacks & ~friendly;
    move.kills = attacks & enemy;
  } break;
  case ROOK: {
    const uint64_t attacks =
        getAttackByOccupancy(square, ROOK_RELEVANT_MASK, ROOK_POSSIBLE_VARIANTS,
                             ROOK_ATTACK_MAP, friendly, enemy);
    move.quiet = attacks & ~friendly;
    move.kills = attacks & enemy;
  } break;
  case QUEEN: {
    const Move rook = getPseudoLegal(ROOK, coord, friendly, isWhite, enemy);
    const Move bishop = getPseudoLegal(BISHOP, coord, friendly, isWhite, enemy);

    move.quiet = rook.quiet | bishop.quiet;
    move.kills = rook.kills | bishop.kills;
  } break;
  case KING:
    move.quiet = KING_ATTACK_MAP[square] & ~blocked;
    move.kills = KING_ATTACK_MAP[square] & enemy;
    break;
  case NOTHING:
    break;
  }

  return move;
}
