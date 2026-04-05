#ifndef SUDOKUGRID_H
#define SUDOKUGRID_H

#include "Cell.h"
#include <vector>

class SudokuGrid {
private:
    Cell grid[9][9];

public:
    SudokuGrid();

    int getValue(int r, int c) const;
    void setValue(int r, int c, int val);
    
    CellState getState(int r, int c) const;
    void setState(int r, int c, CellState state);
    
    bool isFixed(int r, int c) const;
    void setFixed(int r, int c, bool fixed);

    bool isSafe(int r, int c, int num) const;
    void clearNonFixed();
};

#endif // SUDOKUGRID_H
