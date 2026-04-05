#ifndef EVENTLOGGER_H
#define EVENTLOGGER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class EventLogger {
private:
    std::vector<std::string> messages;
    sf::Font font;
    bool fontLoaded;
    size_t maxLines;
    float startX, startY;

public:
    EventLogger(float x, float y, size_t max_lines);
    
    void loadFont();
    void log(const std::string& msg);
    void render(sf::RenderWindow& window);
};

#endif // EVENTLOGGER_H
