#include "Field.h"

Field::Field(sf::Texture& backgroundTexture) 
    : background(backgroundTexture) 

{
    sf::Vector2f backgroundSize(backgroundTexture.getSize());
    background.setScale(
        800.f / background.getTextureRect().size.x,
        600.f / background.getTextureRect().size.y
    );  
             
    sf::Vector2f desiredSize(128.f, 180.f);
    sf::FloatRect bounds = sprite.getLocalBounds();
    sf::Vector2f scale(desiredSize.x / bounds.size.x, desiredSize.y / bounds.size.y);
    sprite.setScale(scale);
}
