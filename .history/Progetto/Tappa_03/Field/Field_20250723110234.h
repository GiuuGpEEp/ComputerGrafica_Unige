#pragma once
#include <SFML/Graphics.hpp>
#include "Slot.h"
#include <vector>

class Field {
public:
    Field(const sf::Texture& fieldTexture,const sf::Texture& monsterTex,
             const sf::Texture& spellTex,
             const sf::Texture& deckTex,
             const sf::Texture& graveTex,
             const sf::Texture& extraTex,
             const sf::Texture& fieldSlotTex);
    void draw(sf::RenderWindow& window, const sf::Vector2i& mousePos) const;

private:
    sf::Sprite background;
    std::vector<Slot> slots;
};
