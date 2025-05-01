#pragma once

#include "bitboard.h"

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

void bake(void);
