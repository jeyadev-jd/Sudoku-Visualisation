#ifndef CELL_H
#define CELL_H

enum class CellState {
    White,   // Empty
    Blue,    // User-inputted starting number
    Yellow,  // Current cell being processed by AI
    Green,   // Validated number
    Red      // Conflict found (triggering a backtrack)
};

class Cell {
public:
    int value;
    CellState state;
    bool isFixed; // True if user input

    Cell() : value(0), state(CellState::White), isFixed(false) {}
};

#endif // CELL_H
