#include "Slot.h"

Slot::Slot(sf::Vector2f position, Type type, sf::Texture& textureRef)
    : sprite(textureRef), type(type)
}