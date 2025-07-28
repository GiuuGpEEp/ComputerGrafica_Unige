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
        FieldSpell
    };

    Slot(sf::Vector2f position, const sf::Texture& texture, Type type, const sf::Vector2f& size);

    void draw(sf::RenderWindow& window, bool hovered, const sf::Transform& transform) const; //Il const indica che questa funzione non modifica lo stato dell'oggetto Slot
    bool isHovered(const sf::Vector2i& mousePos) const;
    void setOccupied(bool status);
    bool isOccupied() const;
    Type getType() const;

private:
    sf::Sprite sprite;
    bool occupied = false;
    Type type;
};
