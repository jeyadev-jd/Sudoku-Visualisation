#include "Solver.h"
#include <thread>
#include <chrono>

Solver::Solver(SudokuGrid& g, std::atomic<bool>& runningFlag, int delay) 
    : grid(g), isRunning(runningFlag), isPaused(false), stepRequested(false), delayMs(delay), useMRV(false), currentDepth(0) {
    stats = {0, 0, 0, 0};
    onUpdate = [](){};
    onLog = [](const std::string&){};
}

void Solver::setCallbacks(UpdateCallback updateCb, LogCallback logCb) {
    onUpdate = updateCb;
    onLog = logCb;
}

void Solver::changeSpeed(int delta) {
    int newDelay = delayMs + delta;
    if (newDelay < 0) newDelay = 0;
    if (newDelay > 1000) newDelay = 1000;
    delayMs = newDelay;
}

bool Solver::start() {
    stats = {0, 0, 0, 0};
    currentDepth = 0;
    
    auto startTime = std::chrono::steady_clock::now();
    bool result = useMRV ? solveMRV() : solve(0, 0);
    auto endTime = std::chrono::steady_clock::now();
    stats.timeElapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    return result;
}

bool Solver::solve(int r, int c) {
    if (!isRunning) return false;

    currentDepth++;
    if (currentDepth > stats.maxDepth) stats.maxDepth = currentDepth;

    // Trigger UI refresh and pause
    auto pauseAndNotify = [&]() {
        onUpdate();
        
        while (isRunning && isPaused && !stepRequested) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (stepRequested) {
            stepRequested = false;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs.load()));
        }
    };

    // Find next empty cell
    if (r == 9) {
        currentDepth--;
        return true; // Reached end
    }
    if (c == 9) {
        bool res = solve(r + 1, 0); // Next row
        currentDepth--;
        return res;
    }

    if (grid.getValue(r, c) != 0) {
        bool res = solve(r, c + 1); // Skip pre-filled
        currentDepth--;
        return res;
    }

    grid.setState(r, c, CellState::Yellow);
    pauseAndNotify();

    for (int num = 1; num <= 9; num++) {
        if (!isRunning) return false;

        stats.stepsTaken++;
        onLog("Trying " + std::to_string(num) + " at [" + std::to_string(r) + "," + std::to_string(c) + "]");

        if (grid.isSafe(r, c, num)) {
            grid.setValue(r, c, num);
            grid.setState(r, c, CellState::Green);
            // onLog("Placed " + std::to_string(num) + " at [" + std::to_string(r) + "," + std::to_string(c) + "], valid.");
            pauseAndNotify();

            if (solve(r, c + 1)) {
                currentDepth--;
                return true;
            }

            // Backtrack
            grid.setValue(r, c, 0);
            grid.setState(r, c, CellState::Red);
            stats.backtracks++;
            // onLog("No valid moves after placing " + std::to_string(num) + " at [" + std::to_string(r) + "," + std::to_string(c) + "]. Backtracking.");
            pauseAndNotify();
        } else {
            // onLog("Conflict for " + std::to_string(num) + " at [" + std::to_string(r) + "," + std::to_string(c) + "]");
        }
    }

    // After trying 1-9 without success
    grid.setState(r, c, CellState::Red);
    stats.backtracks++;
    onLog("Backtracking from [" + std::to_string(r) + "," + std::to_string(c) + "]");
    pauseAndNotify();
    
    grid.setState(r, c, CellState::White);
    pauseAndNotify();
    currentDepth--;
    return false;
}

bool Solver::solveMRV() {
    if (!isRunning) return false;

    currentDepth++;
    if (currentDepth > stats.maxDepth) stats.maxDepth = currentDepth;

    auto pauseAndNotify = [&]() {
        onUpdate();
        while (isRunning && isPaused && !stepRequested) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (stepRequested) stepRequested = false;
        else std::this_thread::sleep_for(std::chrono::milliseconds(delayMs.load()));
    };

    // Find cell with Minimum Remaining Values
    int minVals = 10;
    int bestR = -1;
    int bestC = -1;
    
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (grid.getValue(r, c) == 0) {
                int validCount = 0;
                for (int num = 1; num <= 9; ++num) {
                    if (grid.isSafe(r, c, num)) validCount++;
                }
                if (validCount < minVals) {
                    minVals = validCount;
                    bestR = r;
                    bestC = c;
                }
            }
        }
    }

    if (bestR == -1) { // No empty cells left
        currentDepth--;
        return true; 
    }
    
    if (minVals == 0) { // Dead end
        currentDepth--;
        return false;
    }

    int r = bestR;
    int c = bestC;

    grid.setState(r, c, CellState::Yellow);
    pauseAndNotify();

    for (int num = 1; num <= 9; num++) {
        if (!isRunning) return false;

        if (grid.isSafe(r, c, num)) {
            stats.stepsTaken++;
            grid.setValue(r, c, num);
            grid.setState(r, c, CellState::Green);
            pauseAndNotify();

            if (solveMRV()) {
                currentDepth--;
                return true;
            }

            // Backtrack
            grid.setValue(r, c, 0);
            grid.setState(r, c, CellState::Red);
            stats.backtracks++;
            pauseAndNotify();
        }
    }

    grid.setState(r, c, CellState::White);
    pauseAndNotify();
    currentDepth--;
    return false;
}
