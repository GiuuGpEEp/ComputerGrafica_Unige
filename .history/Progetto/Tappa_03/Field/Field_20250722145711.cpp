#include "Field.h"

Field::Field(sf::Texture& backgroundTexture) 
    : background(backgroundTexture) 

{
    background.setScale(sf::Vector2f(
        800.f / background.getTextureRect().size.x,
        600.f / background.getTextureRect().size.y
    ));
     
    // Inizializzo le dimendioni dei vari slot
    const float slotWidth = 64.f;
    const float slotHeight = 96.f;
    const float spacing = 10.f;
    const float startX = 100.f;
    const float startYTop = 100.f;
    const float startYBottom = 600.f - 100.f - slotHeight;

}
