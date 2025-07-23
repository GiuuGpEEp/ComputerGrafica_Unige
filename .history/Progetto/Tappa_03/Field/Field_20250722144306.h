#pragma once
#include <SFML/Graphics.hpp>
#include "Slot.h"
#include <vector>

class Field {
public:
    Field(sf::Texture& backgroundTexture);
    void draw(sf::RenderWindow& window, const sf::Vector2i& mousePos) const;

private:
    sf::Sprite background;
    std::vector<Slot> slots;
};
