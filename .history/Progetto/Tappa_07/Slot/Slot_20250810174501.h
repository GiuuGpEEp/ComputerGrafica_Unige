#pragma once
#include "../../resources/GameState.h"

#include <SFML/Graphics.hpp>

class Slot {
public:

    Slot(sf::Vector2f position, const sf::Texture& texture, Type type, const sf::Vector2f& size);

    void draw(sf::RenderWindow& window, bool hovered, GameState gamestate, sf::RenderStates states, sf::Color alphaColor) const; //Il const indica che questa funzione non modifica lo stato dell'oggetto Slot
    bool isHovered(const sf::Vector2i& mousePos) const;
    void setOccupied(bool status);
    bool isOccupied() const;
    Type getType() const;
    sf::Vector2f getPosition() const; // Metodo per ottenere la posizione dello slot

private:
    mutable sf::Sprite sprite;
    bool occupied = false;
    Type type;
};
