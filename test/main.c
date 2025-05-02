#include "bitboard.h"
#include "sysifus.h"
#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TESTS_ITERATIONS 100

void printBitboard(uint64_t bitboard) {
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

static pawnTestingContext generatePawnTestingContext(void) {
  pawnTestingContext ctx = {
      (Coordinate){
          (int8_t)(rand() % BOARD_LENGTH),
          (int8_t)(rand() % BOARD_LENGTH),
      },
      .occupancy = 0ULL,
      .isWhite = rand() % 2,
  };

  // Generate random occupancy by a percentage for each square to be set
  for (int8_t square = 0; square < BOARD_AREA; square++) {
    // Because is a probability between of 1/16
    // so there's a 6.25% of chance that a square is set
    // So occuped bits are more spreaded out
    const uint8_t denominator = 16;

    if ((rand() % (denominator + 1)) == 1) {
      ctx.occupancy |= 1ULL << square;
    }
  }

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

// Property: Pawns should not be able to push if on back rank or invalid square
// Property: A pawn should push 1 step if the square is empty
// Property: A pawn should push 2 steps only if on starting rank and path is
// clear
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

Suite *pawnsTests(void) {
  Suite *suite = suite_create("Pawn move generation tests");

  TCase *pawns = tcase_create("Pawn pushes");
  tcase_add_test(pawns, pawnPushesProps);
  tcase_add_test(pawns, pawnCapturesProps);
  suite_add_tcase(suite, pawns);

  return suite;
}

int main(void) {
  srand(time(NULL));

  int number_failed;
  Suite *pawnSuite = pawnsTests();
  SRunner *pawnSuiteRunner = srunner_create(pawnSuite);

  srunner_run_all(pawnSuiteRunner, CK_NORMAL);
  number_failed = srunner_ntests_failed(pawnSuiteRunner);
  srunner_free(pawnSuiteRunner);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
