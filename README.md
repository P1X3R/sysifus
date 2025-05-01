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
   git clone https://github.com/yourusername/sysifus.git
   cd sysifus
2. Compile the project using `make` or your preferred build system.

## How to Contribute
Feel free to fork the repository, submit issues, and create pull requests. Contributions are welcome, especially in areas like:
- Optimizing move generation.
- Expanding the pseudo-legal move generation to include additional pieces or complex scenarios.
- Developing full legal move filtering logic.

## License
This project is licensed under the MIT License - see the LICENSE file for details.
> ***Let me know if you'd like to tweak anything else!***
