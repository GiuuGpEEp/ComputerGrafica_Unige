#pragma once
#include <SFML/Graphics.hpp>
#include "../Slot.h"
#include <vector>

class Field {
public:
    Field(
        const sf::Texture& fieldTexture, 
        const sf::Texture& slotTexture
    );
    void draw(sf::RenderWindow& window, const sf::Vector2i& mousePos) const;

private:
    sf::Sprite background;
    std::vector<Slot> slots;
    void createSlots(const sf::Texture& slotTexture);
};
