#include "bitboard.h"
#include "luts.h"
#include "sysifus.h"
#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TESTS_ITERATIONS 100
#define VERBOSE_LOG

static void printBitboard(uint64_t bitboard) {
  printf("  A B C D E F G H\n");
  for (int8_t rank = BOARD_LENGTH - 1; rank >= 0; rank--) {
    printf("%d ", rank + 1);
    for (int8_t file = 0; file < BOARD_LENGTH; file++) {
      printf("%c ", isSet((Coordinate){rank, file}, bitboard) ? '#' : '.');
    }
    printf("\n");
  }
}

typedef struct {
  Coordinate coord;
  uint64_t occupancy;
  bool isWhite;
} pawnTestingContext;

static uint64_t generateRandomOccupancy(const uint8_t denominator) {
  uint64_t occupancy = 0;

  for (int8_t square = 0; square < BOARD_AREA; square++) {
    // Because is a probability between of 1/16
    // so there's a 6.25% of chance that a square is set
    // So occuped bits are more spreaded out
    const uint8_t denominator = 16;

    if ((rand() % (denominator + 1)) == 1) {
      occupancy |= 1ULL << square;
    }
  }

  return occupancy;
}

static pawnTestingContext generatePawnTestingContext(void) {
  const uint8_t pawnDenominator = 16;
  pawnTestingContext ctx = {
      (Coordinate){
          (int8_t)(rand() % BOARD_LENGTH),
          (int8_t)(rand() % BOARD_LENGTH),
      },
      .occupancy = generateRandomOccupancy(pawnDenominator),
      .isWhite = rand() % 2,
  };

#ifdef VERBOSE_LOG
  printf("=== Test log ===\n");
  printf("- Rank: %d\n", ctx.coord.rank);
  printf("- File: %d\n", ctx.coord.file);
  printf("- Color: %s\n", ctx.isWhite ? "White" : "Black");
  printf("- Occupancy:\n");
  printBitboard(ctx.occupancy);
#endif /* ifdef VERBOSE_LOG */

  return ctx;
}

/*
 * Valid Square: For any valid coordinate, the result should be a subset of
 * valid squares White Pawns:
 *   - On rank 2, should generate 1 or 2 squares forward (if not blocked)
 *   - On other ranks, should generate exactly 1 square forward (if not blocked)
 * Black Pawns:
 *   - On rank 7, should generate 1 or 2 squares forward (if not blocked)
 *   - On other ranks, should generate exactly 1 square forward (if not blocked)
 * Blocking: Pushes should never include blocked squares
 * Edge Cases: Pawns on back rank (rank 1 for black, rank 8 for white) should
 * return 0
 */
START_TEST(pawnPushesProps) {
  for (int i = 0; i < TESTS_ITERATIONS; i++) {
    const pawnTestingContext ctx = generatePawnTestingContext();
    const int8_t backRank = ctx.isWhite ? 7 : 0;
    const uint64_t pushes =
        generatePawnPushes(ctx.coord, ctx.occupancy, ctx.isWhite);
    if (ctx.coord.rank == backRank) {
      ck_assert_uint_eq(pushes, 0);
      continue;
    }

    const int8_t pawnDirection = ctx.isWhite ? 1 : -1;
    const int8_t starterRank = ctx.isWhite ? 1 : 6;
    const Coordinate singlePush = {
        (int8_t)(ctx.coord.rank + pawnDirection),
        ctx.coord.file,
    };
    const Coordinate doublePush = {
        (int8_t)(singlePush.rank + pawnDirection),
        ctx.coord.file,
    };

    if (!isSet(singlePush, ctx.occupancy)) {
      ck_assert_msg(isSet(singlePush, pushes), "Expected singlePush to be set");
    } else {
      ck_assert_msg(!isSet(singlePush, pushes),
                    "Expected singlePush to be blocked");
    }
    if (ctx.coord.rank == starterRank && !isSet(singlePush, ctx.occupancy)) {
      if (!isSet(doublePush, ctx.occupancy)) {
        ck_assert_msg(isSet(doublePush, pushes),
                      "Expected doublePush to be set");
      } else {
        ck_assert_msg(!isSet(doublePush, pushes),
                      "Expected doublePush to be blocked");
      }
    }
  }
}
END_TEST

/*
 * Valid Captures: Should only return squares diagonally forward
 * Color Specific:
 *   - White pawns capture north-east and north-west
 *   - Black pawns capture south-east and south-west
 * Enemy Presence: Result should be subset of enemy squares
 * Edge Files:
 *   - Pawns on file a should only have right captures
 *   - Pawns on file h should only have left captures
 * No Wraparound: Captures should never wrap around board edges
 */
START_TEST(pawnCapturesProps) {
  for (int i = 0; i < TESTS_ITERATIONS; i++) {
    const pawnTestingContext ctx = generatePawnTestingContext();
    const int8_t backRank = ctx.isWhite ? 7 : 0;
    const uint64_t captures =
        generatePawnCaptures(ctx.coord, ctx.occupancy, ctx.isWhite);
    if (ctx.coord.rank == backRank) {
      ck_assert_uint_eq(captures, 0);
      continue;
    }

    const int8_t pawnDirection = ctx.isWhite ? 1 : -1;
    const Coordinate leftKill = {
        (int8_t)(ctx.coord.rank + pawnDirection),
        (int8_t)(ctx.coord.file - 1),
    };
    const Coordinate rightKill = {
        (int8_t)(leftKill.rank),
        (int8_t)(ctx.coord.file + 1),
    };

    if (isCoordValid(leftKill) && isSet(leftKill, ctx.occupancy)) {
      ck_assert(isSet(leftKill, captures));
    }
    if (isCoordValid(rightKill) && isSet(rightKill, ctx.occupancy)) {
      ck_assert(isSet(rightKill, captures));
    }
  }
}
END_TEST

START_TEST(slidingAttackMap) {
  for (int i = 0; i < TESTS_ITERATIONS; i++) {
    const int8_t square = (int8_t)(rand() % BOARD_AREA);
    const uint64_t friendly = generateRandomOccupancy(8);
    const uint64_t enemy = generateRandomOccupancy(8);
    const bool isBishop = rand() % 2;
    const uint64_t *relevantMask = isBishop
                                       ? (const uint64_t *)BISHOP_RELEVANT_MASK
                                       : (const uint64_t *)ROOK_RELEVANT_MASK;
    uint64_t moves;
    if (isBishop) {
      moves = getAttackByOccupancy(square, relevantMask,
                                   (uint16_t)BISHOP_POSSIBLE_VARIANTS,
                                   BISHOP_ATTACK_MAP, friendly, enemy);
    } else {
      moves = getAttackByOccupancy(square, relevantMask,
                                   (uint16_t)ROOK_POSSIBLE_VARIANTS,
                                   ROOK_ATTACK_MAP, friendly, enemy);
    }

#ifdef VERBOSE_LOG
    printf("=== Test log ===\n");
    printf("- Piece: %s\n", isBishop ? "Bishop" : "Rook");
    printf("- Square: %d\n", square);
    printf("- Friendly:\n");
    printBitboard(friendly);
    printf("- Enemy:\n");
    printBitboard(enemy);
    printf("- Moves:\n");
    printBitboard(moves);
#endif /* ifdef VERBOSE_LOG */

    ck_assert_msg((moves & friendly) == 0,
                  "Attacks include friendly pieces for %s at %d",
                  isBishop ? "bishop" : "rook", square);

    if (isBishop) {
      ck_assert_msg(
          (moves & ~BISHOP_ATTACK_MAP[square][0]) == 0,
          "Blocked board has more moves than empty board for %s at %d ",
          isBishop ? "bishop" : "rook", square);
    } else {
      ck_assert_msg(
          (moves & ~ROOK_ATTACK_MAP[square][0]) == 0,
          "Blocked board has more moves than empty board for %s at %d ",
          isBishop ? "bishop" : "rook", square);
    }
  }
}
END_TEST

Suite *moveGeneration(void) {
  Suite *suite = suite_create("Pseudo-legal move generation test suite");

  TCase *pawns = tcase_create("Pawn moves");
  tcase_add_test(pawns, pawnPushesProps);
  tcase_add_test(pawns, pawnCapturesProps);
  suite_add_tcase(suite, pawns);

  TCase *sliding = tcase_create("Sliding moves");
  tcase_add_test(sliding, slidingAttackMap);
  suite_add_tcase(suite, sliding);

  return suite;
}

int main(void) {
  srand(time(NULL));

  int number_failed;
  Suite *pseudoLegal = moveGeneration();
  SRunner *pseudoLegalRunner = srunner_create(pseudoLegal);

  srunner_run_all(pseudoLegalRunner, CK_NORMAL);
  number_failed = srunner_ntests_failed(pseudoLegalRunner);
  srunner_free(pseudoLegalRunner);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
