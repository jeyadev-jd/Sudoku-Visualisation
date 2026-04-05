# Sudoku Visualizer - C++ Documentation

This document provides a comprehensive overview of the **Sudoku Visualizer** project, explaining its architecture, components, and how the core logic integrates with the graphical user interface.

## 1. Overview
The Sudoku Visualizer is an interactive desktop application built using **C++** and **SFML** (Simple and Fast Multimedia Library). It is designed to:
- Generate readable Sudoku puzzles dynamically with varying difficulties.
- Let users input custom numbers into the grid.
- Visualize the sequence of steps an algorithm takes to solve the puzzle in real-time.
- Calculate and display performance statistics such as the number of nodes visited and backtracks performed.

## 2. Architecture & Core Components

The codebase is split strictly utilizing Object-Oriented principles into the following core classes:

### [Cell](file:///home/jeyadev/SudokuVisualizer/include/Cell.h#12-20) ([Cell.h](file:///home/jeyadev/SudokuVisualizer/include/Cell.h))
Represents an individual 1x1 block on the 9x9 grid.
- **Attributes**: 
  - `value`: The number inside the cell (0 if empty).
  - `state`: Colors used to visually signify the algorithm's decisions: `White` (empty), `Blue` (fixed user input), `Yellow` (currently being processed), `Green` (validated), or `Red` (conflict found / backtracking).
  - [isFixed](file:///home/jeyadev/SudokuVisualizer/src/SudokuGrid.cpp#33-38): Identifies whether the cell is a starting condition/user input restricting it from being overwritten.

### [SudokuGrid](file:///home/jeyadev/SudokuVisualizer/include/SudokuGrid.h#12-13) ([SudokuGrid.h](file:///home/jeyadev/SudokuVisualizer/include/SudokuGrid.h), [SudokuGrid.cpp](file:///home/jeyadev/SudokuVisualizer/src/SudokuGrid.cpp))
Acts as the data structure holding the 2D array of [Cell](file:///home/jeyadev/SudokuVisualizer/include/Cell.h#12-20) objects and enforces Sudoku's rules.
- **Key Methods**: 
  - [isSafe(r, c, num)](file:///home/jeyadev/SudokuVisualizer/src/SudokuGrid.cpp#44-66): Core rule-checking logic. Validates whether `num` can be placed at `[r, c]` without violating the row, column, or internal 3x3 block constraints.

### [Solver](file:///home/jeyadev/SudokuVisualizer/include/Solver.h#39-40) ([Solver.h](file:///home/jeyadev/SudokuVisualizer/include/Solver.h), [Solver.cpp](file:///home/jeyadev/SudokuVisualizer/src/Solver.cpp))
Responsible for running the algorithm that logically solves the Sudoku grid. Executed on a separate thread to prevent freezing the UI.
- **Algorithms**:
  - [solve()](file:///home/jeyadev/SudokuVisualizer/src/Solver.cpp#36-116): Standard Depth-First Search Backtracking. It naively checks empty cells top-to-bottom, left-to-right.
  - [solveMRV()](file:///home/jeyadev/SudokuVisualizer/src/Solver.cpp#117-196): Minimum Remaining Values heuristic. It looks for the cell that has the fewest possible valid numbers remaining, significantly optimizing the search space and reducing backtracks.
- **Concurrency & UI Linking**: The solver respects `std::atomic` flags. Methods like `pauseAndNotify()` cause the solver thread to sleep according to a delay, allowing the user to slow down, speed up, or manually step through the algorithm. 

### [EventLogger](file:///home/jeyadev/SudokuVisualizer/src/EventLogger.cpp#4-7) ([EventLogger.h](file:///home/jeyadev/SudokuVisualizer/include/EventLogger.h), [EventLogger.cpp](file:///home/jeyadev/SudokuVisualizer/src/EventLogger.cpp))
A UI utility subclass handling the scrolling text-log area that renders string messages from the UI and Solver, notifying the user about events like algorithm toggles or "Conflict found".

### [Application](file:///home/jeyadev/SudokuVisualizer/src/Application.cpp#11-42) ([Application.h](file:///home/jeyadev/SudokuVisualizer/include/Application.h), [Application.cpp](file:///home/jeyadev/SudokuVisualizer/src/Application.cpp))
The main orchestrator. It manages the SFML window context and loop structure.
- **Event Loop**: Interprets user inputs like left-clicks, numerical keyboard inputs, and control keys.
- **Rendering Loop**: Parses the state of the [SudokuGrid](file:///home/jeyadev/SudokuVisualizer/include/SudokuGrid.h#12-13) and renders square vectors natively on SFML and updates the text/control panels.
- **Puzzle Generation ([generatePuzzle](file:///home/jeyadev/SudokuVisualizer/src/Application.cpp#313-385))**: Fills out the diagonal 3x3 boxes with randomized values (which are mathematically independent), solves the remaining grid natively, and then punches "holes" dependent on the difficulty set by the user.

## 3. Data Flow and Concurrency

To ensure the Visualization stays responsive while the algorithm operates:

1. **User Request**: The user presses `Enter` on the keyboard for the application to solve a puzzle.
2. **Thread Spawning**: The [Application](file:///home/jeyadev/SudokuVisualizer/src/Application.cpp#11-42) instance locks inputs and spawns a specific `std::thread` targeting `solver.start()`.
3. **Execution & Callbacks**: The algorithm starts recursively traversing the grid. Before stepping forward or backwards (backtracking), the algorithm hits `pauseAndNotify()`.
4. **Synchronization**: By checking atomic flags set by the UI layer (e.g., speed variations, or manual pause actions via `Space`), the algorithm thread yields execution (`std::this_thread::sleep_for`), visually extending the process so humans can witness it solving line-by-line while the Main Thread continues to smoothly wipe and redraw the updated grid states to the `window`.

## 4. Building the Project
The application uses standard `CMake` for build generation along with `FetchContent` to natively embed SFML's repository.
```bash
cmake -B build
cmake --build build
./build/SudokuVisualizer
```
*(Note: Requires native system dependencies for compiling SFML natively on Linux, such as X11, Mesa, UDEV and Freetype).*
