#pragma once

#include "bitboard.h"
#include <stdbool.h>
#include <stdint.h>

uint64_t generatePawnPushes(Coordinate coord, uint64_t occupancy, bool isWhite);
