# Sysifus - Pseudo-Legal Move Generation for Chess

Sysifus is a highly optimized and innovative chess move generation system focused on the **pseudo-legal** move generation for chess pieces. This project is built to handle chess moves efficiently, providing a foundation that can later be expanded into a full chess engine.

## Features

- **Pseudo-legal Move Generation**: Efficient and customizable generation of potential moves for all chess pieces.
- **Bitboard Representation**: Uses a compact bitboard representation for the chessboard to minimize memory usage and optimize move generation.
- **Optimized for Speed**: Prioritizes move generation speed, laying the groundwork for a fast and competitive chess engine.
- **Designed for Future Expansion**: While the project focuses on pseudo-legal move generation, it is built with expansion in mind to integrate full legality checks, evaluations, and more complex engine features.

### Core Components

- **Pseudo-Legal Move Generator**: This is the heart of Sysifus, responsible for generating all possible legal moves for each piece type (king, queen, bishop, knight, rook, and pawn).
- **Bitboard Representation**: The board and pieces are represented as 64-bit integers for space efficiency and quick bitwise operations.
- **Piece Movement Logic**: Specific move generation logic for each piece type, handling unique movement patterns and constraints (e.g., knights jumping, bishops moving diagonally).

### Future Expansion

- **Legal Move Generation**: Extend pseudo-legal moves to fully account for check conditions, ensuring that moves that put the player’s king in check are filtered out.
- **UCI Integration**: Eventually integrate with Universal Chess Interface (UCI) for engine functionality.
- **Position Evaluation**: Add heuristics to evaluate positions for chess AI functionality.

> ***In short, make it a full UCI-compatible engine.***

## How to Use

### Building

1. Clone the repository:
   ```bash
   git clone https://github.com/P1X3R/sysifus.git
   cd sysifus
   ```
2. Compile the project using `xmake`
3. You can run the test using `xmake r`

### Generate moves
One code example explains more than two paragraphs of documentation:

```c
#include <sysifus.h>
#include <bitboard.h> // Some bitboard helper functions to make life easier
#include <stdint.h>

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

int main(void) {
  // Example: Generate pseudo-legal moves for a white knight on square b1
  const Coordinate position = {0, 1}; // rank 0, file 1 -> b1
  const uint64_t whitePieces = 1ULL << coordToSquare(position); // bitboard with knight at b1
  const uint64_t blackPieces = 1ULL << coordToSquare((Coordinate){2, 2}); // e.g. enemy piece at c3

  Move moves = getPseudoLegal(KNIGHT, position, whitePieces, true, blackPieces);
  printBitboard(moves.quiet);
  printBitboard(moves.kills);
  
  // Do something with the generated moves...
  return 0;
}
```

## How to Contribute
Feel free to fork the repository, submit issues, and create pull requests. Contributions are welcome, especially in areas like:
- Optimizing move generation.
- Expanding the pseudo-legal move generation to include additional pieces or complex scenarios.
- Developing full legal move filtering logic.

## License
This project is licensed under the MIT License - see the LICENSE file for details.
> ***Sysifus is still evolving — contributions, critiques, and collaborations are all welcome.***
