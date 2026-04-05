#ifndef APPLICATION_H
#define APPLICATION_H

#include <SFML/Graphics.hpp>
#include "SudokuGrid.h"
#include "Solver.h"
#include "EventLogger.h"
#include <atomic>
#include <thread>
#include <memory>

class Application {
private:
    sf::RenderWindow window;
    SudokuGrid grid;
    EventLogger logger;
    
    std::atomic<bool> isSolving;
    std::unique_ptr<std::thread> solverThread;
    Solver solver;

    sf::Font font;
    bool fontLoaded;

    int selectedRow;
    int selectedCol;

    void processEvents();
    void render();
    void handleMouseClick(int x, int y);
    void handleKeyPress(sf::Keyboard::Key key);
    
    void generatePuzzle(int difficulty); // 1=Easy, 2=Med, 3=Hard
    
    void drawGrid();
    void drawControls();

public:
    Application();
    ~Application();
    
    void run();
};

#endif // APPLICATION_H
