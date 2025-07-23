#include "Field.h"

Field::Field(sf::Texture& backgroundTexture) 
    : background(backgroundTexture) {
     
        background.setScale(
        800.f / background.getTextureRect().size.width,
        600.f / background.getTextureRect().height
    );   
}
