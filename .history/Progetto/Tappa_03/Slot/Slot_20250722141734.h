#pragma once
#include <SFML/Graphics.hpp>

class Slot {
public:
    enum class Type {
        Monster,
        SpellTrap,
        Deck,
        Graveyard,
        Extra,
        Field
    };

    Slot(sf::Vector2f position, const sf::Texture& texture, Type type);

    void draw(sf::RenderWindow& window, bool hovered) const;
    bool isHovered(const sf::Vector2i& mousePos) const;
    void setOccupied(bool status);
    bool isOccupied() const;

private:
    sf::Sprite sprite;
    bool occupied = false;
    Type type;
};
