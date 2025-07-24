#include "Slot.h"

Slot::Slot(sf::Vector2f position, const sf::Texture& textureRef, Type type, const sf::Vector2f& size, bool isPlayer2)
    : sprite(textureRef), type(type)
    
{   
    // Scala lo sprite per adattarlo alle dimensioni desiderate dinamiche
    sf::FloatRect bounds = sprite.getLocalBounds();
    sf::Vector2f scale(size.x / bounds.size.x, size.y / bounds.size.y);
    sprite.setScale(scale);
    
    if (isPlayer2) {
        // Per il giocatore 2, ruotiamo di 180 gradi usando sf::degrees
        sprite.setRotation(sf::degrees(180.0f));
        // Aggiustiamo la posizione perché la rotazione cambia l'origine
        sprite.setPosition(sf::Vector2f(position.x + size.x, position.y + size.y));
    } else {
        sprite.setPosition(position);
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

Slot::Type Slot::getType() const {
    return type;
}

