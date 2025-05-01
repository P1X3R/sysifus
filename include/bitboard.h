#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BOARD_LENGTH 8
#define BOARD_AREA 64

typedef struct {
  int8_t rank, file;
} Coordinate;

static inline bool isCoordValid(const Coordinate coord) {
  return coord.rank >= 0 && coord.file >= 0 && coord.rank < BOARD_LENGTH &&
         coord.file < BOARD_LENGTH;
}

static inline int8_t coordToSquare(const Coordinate coord) {
  return (int8_t)((coord.rank * BOARD_LENGTH) + coord.file);
}

static inline bool isSet(const Coordinate coord, const uint64_t bitboard) {
  return (bitboard & (1ULL << coordToSquare(coord)));
}
