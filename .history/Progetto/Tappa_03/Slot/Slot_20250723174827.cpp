#include "Slot.h"

Slot::Slot(sf::Vector2f position, const sf::Texture& textureRef, Type type, const sf::Vector2f& size)
    : sprite(textureRef), type(type)
    
{   
    // Scala lo sprite per adattarlo alle dimensioni desiderate dinamiche
    sf::FloatRect bounds = sprite.getLocalBounds();
    sf::Vector2f scale(size.x / bounds.size.x, size.y / bounds.size.y);
    sprite.setScale(scale);
    
    sprite.setPosition(position);
}

sf::IntRect Slot::getTextureRect(Type type) const {
    // Dimensioni di ogni slot nella texture originale
    const int width = 128;
    const int height = 180;

    // Mapping corretto basato sui problemi che hai identificato:
    // Proviamo una mappatura diversa per correggere i ritagli
    switch (type) {
        case Type::Monster:
            // Giallo - proviamo posizione (2,0) per evitare il "solo spigolo verde"
            return sf::IntRect(sf::Vector2i(2 * width, 0 * height), sf::Vector2i(width, height));
        case Type::SpellTrap:
            // Azzurro - proviamo posizione (3,1) per avere tutto azzurro
            return sf::IntRect(sf::Vector2i(3 * width, 1 * height), sf::Vector2i(width, height));
        case Type::FieldSpell:
            // Verde - proviamo posizione (1,1) per avere tutto verde
            return sf::IntRect(sf::Vector2i(1 * width, 1 * height), sf::Vector2i(width, height));
        case Type::Graveyard:
            // Grigio - proviamo posizione (4,0) 
            return sf::IntRect(sf::Vector2i(4 * width, 0 * height), sf::Vector2i(width, height));
        case Type::Deck:
            // Rosso - proviamo posizione (3,0) per avere tutto rosso
            return sf::IntRect(sf::Vector2i(3 * width, 0 * height), sf::Vector2i(width, height));
        case Type::Extra:
            // Rosso - proviamo posizione (4,1) per ExtraDeck
            return sf::IntRect(sf::Vector2i(4 * width, 1 * height), sf::Vector2i(width, height));
        default:
            return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(width, height));
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

