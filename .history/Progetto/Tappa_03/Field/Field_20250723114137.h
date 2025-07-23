#pragma once
#include <SFML/Graphics.hpp>
#include "Slot.h"
#include <vector>

class Field {
public:
    Field(
        const sf::Texture& fieldTexture, 
        const sf::Texture& monsterTexture,
        const sf::Texture& spellTexture,
        const sf::Texture& deckTexture,
        const sf::Texture& graveTexture,
        const sf::Texture& extraTexture,
        const sf::Texture& fieldSpellTexture,
        const sf::Texture& centerBarTexture
    );
    void draw(sf::RenderWindow& window, const sf::Vector2i& mousePos) const;

private:
    sf::Sprite background;
    std::vector<Slot> slots;
};
