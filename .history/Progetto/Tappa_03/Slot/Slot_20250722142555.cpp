#include "Slot.h"

Slot::Slot(sf::Vector2f position, const sf::Texture& textureRef, Type type)
    : sprite(textureRef), type(type)
    
{
    sprite.setScale(0.5f, 0.5f); // Imposta la scala dello slot (ad esempio, metà della dimensione originale)
    sprite.setPosition(position);
}