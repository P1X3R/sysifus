#pragma once

#include "bitboard.h"
#include <stdbool.h>
#include <stdint.h>

#define JUMPING_OFFSETS 8
#define SLIDING_DIRECTIONS 4
#define BISHOP_POSSIBLE_VARIANTS 512
#define ROOK_POSSIBLE_VARIANTS 4096

static const Coordinate KNIGHT_OFFSETS[JUMPING_OFFSETS] = {
    {+2, +1}, {+2, -1}, {-2, +1}, {-2, -1},
    {+1, +2}, {+1, -2}, {-1, +2}, {-1, -2}};

static const Coordinate KING_OFFSETS[JUMPING_OFFSETS] = {
    {+1, 0},  {-1, 0},  // Vertical
    {0, +1},  {0, -1},  // Horizontal
    {+1, +1}, {+1, -1}, // Diagonal
    {-1, +1}, {-1, -1}};

static const Coordinate BISHOP_DIRECTIONS[SLIDING_DIRECTIONS] = {
    // / Diagonal
    {+1, +1},
    {+1, -1},

    // \ Diagonal
    {-1, +1},
    {-1, -1}};

static const Coordinate ROOK_DIRECTIONS[SLIDING_DIRECTIONS] = {
    {+1, 0}, // Down
    {-1, 0}, // Up
    {0, +1}, // Right
    {0, -1}  // Left
};

uint64_t generatePawnPushes(Coordinate coord, uint64_t blockedSquares,
                            bool isWhite);
uint64_t generatePawnCaptures(Coordinate coord, uint64_t enemy, bool isWhite);

inline uint64_t getAttacksByLUT(const uint64_t lut[BOARD_AREA],
                                const int8_t square,
                                const uint64_t blockedSquare) {
  // Add bounds check since this is a public function
  if (square < 0 || square >= BOARD_AREA) {
    return 0;
  }

  return lut[square] & ~blockedSquare;
}

uint64_t getAttackByOccupancy(int8_t square,
                              const uint64_t relevantMask[BOARD_AREA],
                              uint16_t possibleVariants,
                              const uint64_t lut[BOARD_AREA][possibleVariants],
                              uint64_t friendly, uint64_t enemy);

void bake(void);
