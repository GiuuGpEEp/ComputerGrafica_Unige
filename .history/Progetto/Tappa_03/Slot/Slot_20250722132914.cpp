#include "Slot.h"

Slot::Slot(sf::Vector2f position, sf::Vector2f size, sf::Texture& textureRef) {
    // Inizializzo lo sprite del slot con la texture fornita
    sprite.setTexture(textureRef);
    
    // Imposto le dimensioni dello sprite dello slot
    sprite.setScale(size.x / sprite.getLocalBounds().size.x, size.y / sprite.getLocalBounds().height);
    
    // Imposto la posizione dello slot
    sprite.setPosition(position);
}