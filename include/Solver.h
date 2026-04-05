#ifndef SOLVER_H
#define SOLVER_H

#include "SudokuGrid.h"
#include <functional>
#include <atomic>
#include <string>

// Callback types for UI integration
using UpdateCallback = std::function<void()>;
using LogCallback = std::function<void(const std::string&)>;

class Solver {
public:
    struct Stats {
        int stepsTaken;
        int maxDepth;
        int backtracks;
        long long timeElapsedMs;
    };

private:
    SudokuGrid& grid;
    UpdateCallback onUpdate;
    LogCallback onLog;
    std::atomic<bool>& isRunning;
    std::atomic<bool> isPaused;
    std::atomic<bool> stepRequested;
    std::atomic<int> delayMs;
    std::atomic<bool> useMRV;
    
    Stats stats;
    int currentDepth;

    bool solve(int r, int c);
    bool solveMRV();

public:
    Solver(SudokuGrid& g, std::atomic<bool>& runningFlag, int delay = 100);
    
    void setCallbacks(UpdateCallback updateCb, LogCallback logCb);
    
    void setDelay(int ms) { delayMs = ms; }
    int getDelay() const { return delayMs; }
    void changeSpeed(int delta);
    
    void togglePause() { isPaused = !isPaused; }
    bool getPaused() const { return isPaused; }
    void requestStep() { stepRequested = true; }
    
    void setUseMRV(bool mrv) { useMRV = mrv; }
    bool isUsingMRV() const { return useMRV; }
    
    // Starts the recursive backtracking
    bool start();
    const Stats& getStats() const { return stats; }
};

#endif // SOLVER_H
