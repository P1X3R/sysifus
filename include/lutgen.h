#pragma once

#include "bitboard.h"
#include <stdint.h>

#define JUMPING_OFFSETS 8
#define SLIDING_DIRECTIONS 4
#define BISHOP_POSSIBLE_VARIANTS 512
#define ROOK_POSSIBLE_VARIANTS 4096

const Coordinate knightOffsets[JUMPING_OFFSETS] = {{+2, +1}, {+2, -1}, {-2, +1},
                                                   {-2, -1}, {+1, +2}, {+1, -2},
                                                   {-1, +2}, {-1, -2}};

const Coordinate kingOffsets[JUMPING_OFFSETS] = {
    {+1, 0},  {-1, 0},  // Vertical
    {0, +1},  {0, -1},  // Horizontal
    {+1, +1}, {+1, -1}, // Diagonal
    {-1, +1}, {-1, -1}};

const Coordinate bishopDirections[SLIDING_DIRECTIONS] = {
    // / Diagonal
    {+1, +1},
    {+1, -1},

    // \ Diagonal
    {-1, +1},
    {-1, -1}};

const Coordinate rookDirections[SLIDING_DIRECTIONS] = {
    {+1, 0}, // Down
    {-1, 0}, // Up
    {0, +1}, // Right
    {0, -1}  // Left
};

void generateJumpingAttackMap(const Coordinate offsets[JUMPING_OFFSETS],
                              uint64_t lut[BOARD_LENGTH]);
void generateSlidingAttackMap(const Coordinate directions[SLIDING_DIRECTIONS],
                              uint64_t lut[BOARD_LENGTH]);
void generateSlidingAttackMapOccupancy(
    const int8_t directions[SLIDING_DIRECTIONS], uint8_t possibleVariants,
    uint64_t lut[BOARD_LENGTH][possibleVariants]);
