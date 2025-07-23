#include "Slot.h"

Slot::Slot(sf::Vector2f position, const sf::Texture& textureRef, Type type)
    : sprite(textureRef), type(type)
    
{    
    // Imposto le dimensioni desiderate per lo sprite della carta in modo dinamico
    sf::Vector2f desiredSize(133.f, 185.f);
    sf::FloatRect bounds = sprite.getLocalBounds();
    sf::Vector2f scale(desiredSize.x / bounds.size.x, desiredSize.y / bounds.size.y);
    sprite.setScale(scale);

    sprite.setPosition(position);
}

void Slot::draw(sf::RenderWindow& window, bool hovered) const {
    window.draw(sprite);

    if(hovered){
        sf::RectangleShape hoverBorder;
        hoverBorder.setSize(sprite.getGlobalBounds().getSize());
        hoverBorder.setPosition(sprite.getGlobalBounds().getPosition());
        hoverBorder.setOutlineColor(sf::Color::Yellow);
        hoverBorder.setOutlineThickness(5.f);
        hoverBorder.setFillColor(sf::Color::Transparent);
        window.draw(hoverBorder);
    }
}