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

    // 3 mostri sopra, 3 magie sotto per ciascun lato
    for (int i = 0; i < 3; ++i) {
        // Player 1 mostri
        slots.emplace_back(sf::Vector2f(startX + i * (slotWidth + spacing), startYTop), sf::Vector2f(slotWidth, slotHeight));
        // Player 1 magie/trappole
        slots.emplace_back(sf::Vector2f(startX + i * (slotWidth + spacing), startYTop + slotHeight + spacing), sf::Vector2f(slotWidth, slotHeight));

        // Player 2 magie/trappole
        slots.emplace_back(sf::Vector2f(startX + i * (slotWidth + spacing), startYBottom - slotHeight - spacing), sf::Vector2f(slotWidth, slotHeight));
        // Player 2 mostri
        slots.emplace_back(sf::Vector2f(startX + i * (slotWidth + spacing), startYBottom), sf::Vector2f(slotWidth, slotHeight));
    }
}
