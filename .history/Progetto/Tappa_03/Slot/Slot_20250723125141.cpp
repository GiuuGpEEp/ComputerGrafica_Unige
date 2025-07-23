#include "Slot.h"

Slot::Slot(sf::Vector2f position, const sf::Texture& textureRef, Type type)
    : sprite(textureRef), type(type)
    
{   
    sprite.setTextureRect(getTextureRect(type)); // Imposto il rettangolo di texture in base al tipo di slot
    sprite.setPosition(position);
}

sf::IntRect Slot::getTextureRect(Type type) const {
    //Suppongo che ogni slot sia di dimensioni fisse, ad esempio 128x180
    const int width = 128;
    const int height = 180;

    switch (type) {
        case Type::Monster:
            return sf::IntRect(sf::Vector2i(1 * width, 0 * height), sf::Vector2i(width, height));
        case Type::SpellTrap:
            return sf::IntRect(sf::Vector2i(1 * width, 1 * height), sf::Vector2i(width, height));
        case Type::Deck:
            return sf::IntRect(sf::Vector2i(2 * width, 0 * height), sf::Vector2i(width, height));
        case Type::Graveyard:
            return sf::IntRect(sf::Vector2i(2 * width, 1 * height), sf::Vector2i(width, height));
        case Type::Extra:
            return sf::IntRect(sf::Vector2i(3 * width, 0 * height), sf::Vector2i(width, height));
        case Type::FieldSpell:
            return sf::IntRect(sf::Vector2i(3 * width, 1 * height), sf::Vector2i(width, height));
        default:
            return sf::IntRect(); // Restituisce un rettangolo vuoto se il    


            
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