#include "SudokuGrid.h"

SudokuGrid::SudokuGrid() {
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            grid[r][c] = Cell();
        }
    }
}

int SudokuGrid::getValue(int r, int c) const {
    if (r >= 0 && r < 9 && c >= 0 && c < 9)
        return grid[r][c].value;
    return 0;
}

void SudokuGrid::setValue(int r, int c, int val) {
    if (r >= 0 && r < 9 && c >= 0 && c < 9)
        grid[r][c].value = val;
}

CellState SudokuGrid::getState(int r, int c) const {
    if (r >= 0 && r < 9 && c >= 0 && c < 9)
        return grid[r][c].state;
    return CellState::White;
}

void SudokuGrid::setState(int r, int c, CellState state) {
    if (r >= 0 && r < 9 && c >= 0 && c < 9)
        grid[r][c].state = state;
}

bool SudokuGrid::isFixed(int r, int c) const {
    if (r >= 0 && r < 9 && c >= 0 && c < 9)
        return grid[r][c].isFixed;
    return false;
}

void SudokuGrid::setFixed(int r, int c, bool fixed) {
    if (r >= 0 && r < 9 && c >= 0 && c < 9)
        grid[r][c].isFixed = fixed;
}

bool SudokuGrid::isSafe(int r, int c, int num) const {
    // Check row
    for (int i = 0; i < 9; i++) {
        if (i != c && grid[r][i].value == num) return false;
    }
    // Check col
    for (int i = 0; i < 9; i++) {
        if (i != r && grid[i][c].value == num) return false;
    }
    // Check 3x3 box
    int startRow = r - r % 3;
    int startCol = c - c % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if ((i + startRow != r || j + startCol != c) && 
                grid[i + startRow][j + startCol].value == num) {
                return false;
            }
        }
    }
    return true;
}

void SudokuGrid::clearNonFixed() {
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (!grid[r][c].isFixed) {
                grid[r][c].value = 0;
                grid[r][c].state = CellState::White;
            } else {
                grid[r][c].state = CellState::Blue; // reset user numbers to blue
            }
        }
    }
}
