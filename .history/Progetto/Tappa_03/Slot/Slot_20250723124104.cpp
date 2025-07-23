#include "Slot.h"

Slot::Slot(sf::Vector2f position, const sf::Texture& textureRef, Type type)
    : sprite(textureRef), type(type)
    
{   
    sprite.setTextureRect(getTextureRect(type)); // Imposto il rettangolo di texture in base al tipo di slot
    sprite.setPosition(position);
}

sf::IntRect Slot::getTextureRect(Type type) const {
    switch (type) {
        case Type::Monster:
            return  // Esempio di rettangolo per mostro
        case Type::SpellTrap:
            return sf::IntRect(100, 0, 100, 150); // Esempio di rettangolo per magia/trappola
        case Type::Deck:
            return sf::IntRect(200, 0, 100, 150); // Esempio di rettangolo per mazzo
        case Type::Graveyard:
            return sf::IntRect(300, 0, 100, 150); // Esempio di rettangolo per cimitero
        case Type::Extra:
            return sf::IntRect(400, 0, 100, 150); // Esempio di rettangolo per extra deck
        case Type::FieldSpell:
            return sf::IntRect(500, 0, 100, 150); // Esempio di rettangolo per campo magico
        default:
            return sf::IntRect(); // Rettangolo vuoto se il tipo non è riconosciuto
    }
}



void Slot::draw(sf::RenderWindow& window, bool hovered) const {
    window.draw(sprite);

    if(hovered){ //Se il mouse è sopra lo slot, disegno un bordo giallo
        sf::RectangleShape hoverBorder;
        hoverBorder.setSize({sprite.getGlobalBounds().size.x, sprite.getGlobalBounds().size.y});
        hoverBorder.setPosition(sprite.getPosition());
        hoverBorder.setOutlineColor(sf::Color::Yellow);
        hoverBorder.setOutlineThickness(5.f);
        hoverBorder.setFillColor(sf::Color::Transparent);
        window.draw(hoverBorder);
    }
}

bool Slot::isHovered(const sf::Vector2i& mousePos) const {
    return sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

void Slot::setOccupied(bool status) {
    occupied = status; // Imposta lo stato di occupazione dello slot
}

bool Slot::isOccupied() const {
    return occupied;
}