#pragma once
#include <SFML/Graphics.hpp>

class Slot {
public:
    Slot(sf::Vector2f position, sf::Vector2f size);

    void draw(sf::RenderWindow& window, bool hovered) const;
    bool isHovered(const sf::Vector2i& mousePos) const;
    void setOccupied(bool status);
    bool isOccupied() const;

private:
    bool occupied = false;
    sf::Sprite sprite;
};
