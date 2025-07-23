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
    if (hovered) {
        // Disegna lo sprite con un bordo o un effetto di hover
        sf::Color hoverColor = sf::Color(255, 255, 255, 150); // Colore semi-trasparente per l'hover
        sprite.setColor(hoverColor);
    } else {
        sprite.setColor(sf::Color::White); // Ripristina il colore normale
    }
    window.draw(sprite);
}