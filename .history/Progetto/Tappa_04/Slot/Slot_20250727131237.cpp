#include "Slot.h"

Slot::Slot(sf::Vector2f position, const sf::Texture& textureRef, Type type, const sf::Vector2f& size, const sf::Texture& textureRef)
    : sprite(textureRef), type(type)
    
{   
    // Scala lo sprite per adattarlo alle dimensioni desiderate dinamiche
    sf::FloatRect bounds = sprite.getLocalBounds();
    sf::Vector2f scale(size.x / bounds.size.x, size.y / bounds.size.y);
    sprite.setScale(scale);
    
    sprite.setPosition(position);
}

//Aggiungere sf::Transform all'interno di slot
void Slot::draw(sf::RenderWindow& window, bool hovered) const {
    sf::RenderStates states;
    states.transform = transform; // Applica la trasformazione passata come parametro
    window.draw(sprite, states);

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

Slot::Type Slot::getType() const {
    return type;
}

