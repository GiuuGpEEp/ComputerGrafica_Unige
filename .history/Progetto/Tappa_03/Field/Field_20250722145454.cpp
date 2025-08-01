#include "Field.h"

Field::Field(sf::Texture& backgroundTexture) 
    : background(backgroundTexture) 

{
    background.setScale(sf::Vector2f(
        800.f / background.getTextureRect().size.x,
        600.f / background.getTextureRect().size.y
    ));
}
