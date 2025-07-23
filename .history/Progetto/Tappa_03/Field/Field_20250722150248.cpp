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

    // Deck, Extra, Graveyard
    float sideSlotY1 = startYTop + 2 * (slotHeight + spacing);
    float sideSlotY2 = startYBottom - 2 * (slotHeight + spacing);

    float leftX = 30.f;
    float rightX = 700.f;

     // Player 1
    slots.emplace_back(sf::Vector2f(leftX, sideSlotY1), sf::Vector2f(slotWidth, slotHeight)); // Deck
    slots.emplace_back(sf::Vector2f(leftX + 70.f, sideSlotY1), sf::Vector2f(slotWidth, slotHeight)); // Cimitero
    slots.emplace_back(sf::Vector2f(leftX + 140.f, sideSlotY1), sf::Vector2f(slotWidth, slotHeight)); // Extra
    slots.emplace_back(sf::Vector2f(leftX + 210.f, sideSlotY1), sf::Vector2f(slotWidth, slotHeight)); // Terreno

    // Player 2
    slots.emplace_back(sf::Vector2f(leftX, sideSlotY2), sf::Vector2f(slotWidth, slotHeight)); // Deck
    slots.emplace_back(sf::Vector2f(leftX + 70.f, sideSlotY2), sf::Vector2f(slotWidth, slotHeight)); // Cimitero
    slots.emplace_back(sf::Vector2f(leftX + 140.f, sideSlotY2), sf::Vector2f(slotWidth, slotHeight)); // Extra
    slots.emplace_back(sf::Vector2f(leftX + 210.f, sideSlotY2), sf::Vector2f(slotWidth, slotHeight)); // Terreno

}

//Continuare con il draw
