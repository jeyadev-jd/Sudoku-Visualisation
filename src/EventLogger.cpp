#include "EventLogger.h"
#include <iostream>

EventLogger::EventLogger(float x, float y, size_t max_lines) 
    : startX(x), startY(y), maxLines(max_lines), fontLoaded(false) {
}

void EventLogger::loadFont() {
    if (font.loadFromFile("C:\\Windows\\Fonts\\consola.ttf") || // Windows Default
        font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf") || // Windows Alternative
        font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf") || // Linux Default
        font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf") || 
        font.loadFromFile("arial.ttf")) { // Local fallback
        fontLoaded = true;
    } else {
        std::cerr << "Failed to load fonts for Event Logger." << std::endl;
    }
}

void EventLogger::log(const std::string& msg) {
    if (messages.size() >= maxLines) {
        messages.erase(messages.begin());
    }
    messages.push_back(msg);
}

void EventLogger::render(sf::RenderWindow& window) {
    if (!fontLoaded) return;

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(14);
    text.setFillColor(sf::Color::Black);

    float currentY = startY;
    for (const auto& msg : messages) {
        text.setString(msg);
        text.setPosition(startX, currentY);
        window.draw(text);
        currentY += 18.0f; // line height
    }
}
