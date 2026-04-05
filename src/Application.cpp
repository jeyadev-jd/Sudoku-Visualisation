#include "Application.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

const float CELL_SIZE = 60.0f;
const float GRID_OFFSET_X = 50.0f;
const float GRID_OFFSET_Y = 50.0f;

Application::Application() 
    : window(sf::VideoMode(800, 800), "Sudoku Visualizer"),
      logger(50.0f, 620.0f, 8),
      isSolving(false),
      solver(grid, isSolving, 50),
      selectedRow(-1), selectedCol(-1) {
    
    window.setFramerateLimit(60);
    logger.loadFont();
    
    if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf") || // Windows Default
        font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") || // Linux Default
        font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf") ||
        font.loadFromFile("arial.ttf")) { // Local fallback
        fontLoaded = true;
    } else {
        fontLoaded = false;
        std::cerr << "Failed to load font for grid numbers." << std::endl;
    }

    // Set up solver callbacks
    solver.setCallbacks(
        [this]() { 
            // We shouldn't call window.display() from another thread directly,
            // but for a simple visualizer, we can flag a redraw or simply 
            // let the main thread render loop pick up the state changes.
        },
        [this](const std::string& msg) {
            logger.log(msg);
        }
    );
}

Application::~Application() {
    isSolving = false;
    if (solverThread && solverThread->joinable()) {
        solverThread->join();
    }
}

void Application::run() {
    while (window.isOpen()) {
        processEvents();
        render();
    }
}

void Application::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        
        if (isSolving) continue; // Block input while solving

        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                handleMouseClick(event.mouseButton.x, event.mouseButton.y);
            }
        } else if (event.type == sf::Event::KeyPressed) {
            handleKeyPress(event.key.code);
        }
    }
}

void Application::handleMouseClick(int x, int y) {
    if (x >= GRID_OFFSET_X && x < GRID_OFFSET_X + 9 * CELL_SIZE &&
        y >= GRID_OFFSET_Y && y < GRID_OFFSET_Y + 9 * CELL_SIZE) {
        selectedCol = (x - GRID_OFFSET_X) / CELL_SIZE;
        selectedRow = (y - GRID_OFFSET_Y) / CELL_SIZE;
    } else {
        selectedRow = -1;
        selectedCol = -1;
    }
}

void Application::handleKeyPress(sf::Keyboard::Key key) {
    // Solve button (Enter)
    if (key == sf::Keyboard::Enter) {
        if (!isSolving) {
            isSolving = true;
            logger.log("Starting solver...");
            
            // Clear previous simulation data
            grid.clearNonFixed();

            solverThread = std::make_unique<std::thread>([this]() {
                bool success = solver.start();
                if (success) {
                    logger.log("Solved successfully! Nodes: " + std::to_string(solver.getStats().stepsTaken));
                } else {
                    logger.log("Unsolvable puzzle.");
                }
                isSolving = false;
            });
        }
        return;
    }

    // Reset button (R)
    if (key == sf::Keyboard::R) {
        if (!isSolving) {
            isSolving = false;
            if (solverThread && solverThread->joinable()) {
                solverThread->join();
            }
            grid = SudokuGrid();
            logger.log("Grid reset by user.");
        }
        return;
    }

    // Speed control
    if (key == sf::Keyboard::Equal || key == sf::Keyboard::Add) {
        solver.changeSpeed(-10); // faster = less delay
        logger.log("Delay decreased to " + std::to_string(solver.getDelay()) + "ms");
        return;
    }
    if (key == sf::Keyboard::Dash || key == sf::Keyboard::Subtract) {
        solver.changeSpeed(10);
        logger.log("Delay increased to " + std::to_string(solver.getDelay()) + "ms");
        return;
    }
    
    // Manual Step Mode
    if (key == sf::Keyboard::Space) {
        solver.togglePause();
        logger.log(solver.getPaused() ? "Paused" : "Resumed");
        return;
    }
    if (key == sf::Keyboard::Right) {
        if (!solver.getPaused()) {
            solver.togglePause();
            logger.log("Paused automode. Stepping manually.");
        }
        solver.requestStep();
        return;
    }

    // Toggle MRV Algorithm
    if (key == sf::Keyboard::M) {
        if (!isSolving) {
            solver.setUseMRV(!solver.isUsingMRV());
            logger.log("Algorithm changed to: " + std::string(solver.isUsingMRV() ? "MRV" : "Standard Backtracking"));
        }
        return;
    }

    // Number input
    if (selectedRow != -1 && selectedCol != -1) {
        int num = -1;
        if (key >= sf::Keyboard::Num1 && key <= sf::Keyboard::Num9) {
            num = key - sf::Keyboard::Num0;
        } else if (key >= sf::Keyboard::Numpad1 && key <= sf::Keyboard::Numpad9) {
            num = key - sf::Keyboard::Numpad0;
        } else if (key == sf::Keyboard::Num0 || key == sf::Keyboard::Numpad0 || key == sf::Keyboard::Backspace || key == sf::Keyboard::Delete) {
            num = 0; // Clear cell
        }

        if (num != -1) {
            if (num == 0) {
                grid.setValue(selectedRow, selectedCol, 0);
                grid.setState(selectedRow, selectedCol, CellState::White);
                grid.setFixed(selectedRow, selectedCol, false);
            } else if (grid.isSafe(selectedRow, selectedCol, num)) {
                grid.setValue(selectedRow, selectedCol, num);
                grid.setState(selectedRow, selectedCol, CellState::Blue);
                grid.setFixed(selectedRow, selectedCol, true);
            } else {
                logger.log("Conflict: Cannot place " + std::to_string(num) + " at [" + std::to_string(selectedRow) + "," + std::to_string(selectedCol) + "]");
                grid.setState(selectedRow, selectedCol, CellState::Red);
                grid.setValue(selectedRow, selectedCol, num); // Show the conflicting number momentarily
                grid.setFixed(selectedRow, selectedCol, false); // Keep it unfixed so user can edit it
            }
        }
    }
    
    // Puzzle Generation
    if (!isSolving) {
        if (key == sf::Keyboard::Z) generatePuzzle(1);
        if (key == sf::Keyboard::X) generatePuzzle(2);
        if (key == sf::Keyboard::C) generatePuzzle(3);
    }
}

void Application::drawGrid() {
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(GRID_OFFSET_X + c * CELL_SIZE, GRID_OFFSET_Y + r * CELL_SIZE);
            cell.setOutlineThickness(1.0f);
            cell.setOutlineColor(sf::Color(200, 200, 200));

            // Background color based on state
            CellState state = grid.getState(r, c);
            switch (state) {
                case CellState::White:  cell.setFillColor(sf::Color::White); break;
                case CellState::Blue:   cell.setFillColor(sf::Color(173, 216, 230)); break; // Light Blue
                case CellState::Yellow: cell.setFillColor(sf::Color(255, 255, 153)); break; // Light Yellow
                case CellState::Green:  cell.setFillColor(sf::Color(144, 238, 144)); break; // Light Green
                case CellState::Red:    cell.setFillColor(sf::Color(255, 182, 193)); break; // Light Red
            }

            // Highlight selected cell
            if (r == selectedRow && c == selectedCol) {
                cell.setOutlineThickness(3.0f);
                cell.setOutlineColor(sf::Color::Blue);
            }

            window.draw(cell);

            // Draw number
            int val = grid.getValue(r, c);
            if (val != 0 && fontLoaded) {
                sf::Text text;
                text.setFont(font);
                text.setString(std::to_string(val));
                text.setCharacterSize(30);
                text.setFillColor(grid.isFixed(r, c) ? sf::Color(0, 0, 139) : sf::Color::Black);
                
                // Center text
                sf::FloatRect textRect = text.getLocalBounds();
                text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
                text.setPosition(cell.getPosition().x + CELL_SIZE/2.0f, cell.getPosition().y + CELL_SIZE/2.0f);
                window.draw(text);
            }
        }
    }

    // Draw thick 3x3 borders
    for (int i = 0; i <= 9; i += 3) {
        sf::RectangleShape vLine(sf::Vector2f(3.0f, 9 * CELL_SIZE));
        vLine.setFillColor(sf::Color::Black);
        vLine.setPosition(GRID_OFFSET_X + i * CELL_SIZE - 1.5f, GRID_OFFSET_Y);
        window.draw(vLine);

        sf::RectangleShape hLine(sf::Vector2f(9 * CELL_SIZE, 3.0f));
        hLine.setFillColor(sf::Color::Black);
        hLine.setPosition(GRID_OFFSET_X, GRID_OFFSET_Y + i * CELL_SIZE - 1.5f);
        window.draw(hLine);
    }
}

void Application::drawControls() {
    if (!fontLoaded) return;
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(16);
    text.setFillColor(sf::Color::Black);
    text.setPosition(GRID_OFFSET_X + 9 * CELL_SIZE + 20.0f, GRID_OFFSET_Y);
    
    std::string instructions = 
        "Controls:\n\n"
        "Click cell to select.\n"
        "Type 1-9 to input.\n"
        "Type 0, Backspace to clear.\n\n"
        "[Enter] : Solve\n"
        "[R]     : Reset Grid\n\n"
        "[Space] : Pause / Resume\n"
        "[Right] : Step Forward\n"
        "[+/-]   : Change Speed\n\n"
        "[Z/X/C] : Generate Puzzle\n"
        "          (Easy/Med/Hard)\n"
        "[M]     : Toggle MRV (" + std::string(solver.isUsingMRV() ? "ON" : "OFF") + ")\n\n";
        
    if (isSolving) {
        instructions += "Status: " + std::string(solver.getPaused() ? "PAUSED" : "SOLVING...");
    } else {
        instructions += "Status: IDLE";
    }
    
    // Draw stats dashboard
    auto stats = solver.getStats();
    instructions += "\n\n--- Statistics ---\n";
    instructions += "Delay: " + std::to_string(solver.getDelay()) + " ms\n";
    if (isSolving || stats.stepsTaken > 0) {
        instructions += "Nodes: " + std::to_string(stats.stepsTaken) + "\n";
        instructions += "Backtracks: " + std::to_string(stats.backtracks) + "\n";
        instructions += "Max Depth: " + std::to_string(stats.maxDepth) + "\n";
        
        long long ms = stats.timeElapsedMs;
        if (isSolving) {
            // we aren't tracking real time cleanly in the solver while solving dynamically yet
            // but we can just show the end time.
            instructions += "Timing done at end.\n";
        } else {
            char timeBuf[32];
            snprintf(timeBuf, sizeof(timeBuf), "%.3f", static_cast<double>(ms) / 1000.0);
            instructions += "Time: " + std::string(timeBuf) + " sec\n";
        }
    }

    text.setString(instructions);
    window.draw(text);
}

void Application::render() {
    window.clear(sf::Color(240, 240, 240)); // light gray background

    drawGrid();
    drawControls();
    logger.render(window);

    window.display();
}

void Application::generatePuzzle(int difficulty) {
    grid = SudokuGrid(); // clear
    logger.log("Generating puzzle...");

    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Fast inline solver to populate a full grid
    std::function<bool(int, int)> fillGrid = [&](int r, int c) -> bool {
        if (r == 9) return true;
        if (c == 9) return fillGrid(r + 1, 0);
        if (grid.getValue(r, c) != 0) return fillGrid(r, c + 1);

        std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::shuffle(nums.begin(), nums.end(), gen);

        for (int num : nums) {
            if (grid.isSafe(r, c, num)) {
                grid.setValue(r, c, num);
                if (fillGrid(r, c + 1)) return true;
                grid.setValue(r, c, 0);
            }
        }
        return false;
    };

    // Fill diagonal 3x3 blocks first because they're independent
    for (int i = 0; i < 9; i += 3) {
        std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::shuffle(nums.begin(), nums.end(), gen);
        int idx = 0;
        for (int r = i; r < i + 3; ++r) {
            for (int c = i; c < i + 3; ++c) {
                grid.setValue(r, c, nums[idx++]);
            }
        }
    }

    fillGrid(0, 0); // Recursively fill the rest

    // Dig holes based on difficulty
    int holes = 30; // base (Easy)
    if (difficulty == 2) holes = 45;
    if (difficulty == 3) holes = 55;

    std::vector<std::pair<int, int>> positions;
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            positions.push_back({r, c});
        }
    }
    std::shuffle(positions.begin(), positions.end(), gen);

    for (int i = 0; i < holes; ++i) {
        auto pos = positions[i];
        grid.setValue(pos.first, pos.second, 0);
    }

    // Set remaining as fixed
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (grid.getValue(r, c) != 0) {
                grid.setState(r, c, CellState::Blue);
                grid.setFixed(r, c, true);
            } else {
                grid.setState(r, c, CellState::White);
                grid.setFixed(r, c, false);
            }
        }
    }
    logger.log("Generated " + std::string(difficulty == 1 ? "Easy" : (difficulty == 2 ? "Medium" : "Hard")) + " puzzle.");
}

