#include "Field.h"

Field::Field(sf::Texture& backgroundTexture) 
    : background(backgroundTexture) 

{
    background.setScale(
        800.f / background.getLocalBounds().width,
        600.f / background.getLocalBounds().height
    );
}
