#include "sysifus.h"
#include "bitboard.h"
#include <stdint.h>

uint64_t generatePawnPushes(const Coordinate coord, const uint64_t occupancy,
                            const bool isWhite) {
  if (!isCoordValid(coord)) {
    return 0;
  }

  uint64_t pushes = 1ULL << coordToSquare(coord);

  if (isWhite) {
    pushes = (pushes << BOARD_LENGTH) & ~occupancy;

    const int8_t starterRank = 1;
    if (coord.rank == starterRank) {
      pushes |= (pushes << BOARD_LENGTH) & ~occupancy;
    }
  } else {
    pushes = (pushes >> BOARD_LENGTH) & ~occupancy;

    const int8_t starterRank = 6;
    if (coord.rank == starterRank) {
      pushes |= (pushes >> BOARD_LENGTH) & ~occupancy;
    }
  }

  return pushes;
}
