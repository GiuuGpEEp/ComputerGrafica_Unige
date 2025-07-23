#include "Slot.h"

Slot::Slot(sf::Vector2f position, const sf::Texture& textureRef, Type type)
    : sprite(textureRef), type(type)
    
{    
    // Imposto le dimensioni desiderate per lo sprite della carta in modo dinamico
    sf::Vector2f desiredSize(1336.f, 185.f);
    sf::FloatRect bounds = sprite.getLocalBounds();
    sf::Vector2f scale(desiredSize.x / bounds.size.x, desiredSize.y / bounds.size.y);
    sprite.setScale(scale);

    sprite.setPosition(position);
}