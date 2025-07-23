#pragma once
#include <SFML/Graphics.hpp>
#include "Slot.h"
#include <vector>

class Field {
public:
    Field(
        const sf::Texture& fieldTexture, 
        const sf::Texture& slotTexture,
        const sf::Texture& centerBarTexture
    );
    void draw(sf::RenderWindow& window, const sf::Vector2i& mousePos, const sf::Texture& centerBarTexture) const;

private:
    sf::Sprite background;
    std::vector<Slot> slots;
    sf::Sprite centerBar;
    void createSlots(const sf::Texture& slotTexture);
};
