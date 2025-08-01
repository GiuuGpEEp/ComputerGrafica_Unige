#include "Field.h"

Field::Field(
            const sf::Texture& fieldTexture, 
            const sf::Texture& slotTexture,
            const sf::Texture& centerBarTexture
        ) : background(fieldTexture), centerBar(centerBarTexture)
{
    background.setScale(sf::Vector2f(
        800.f / background.getTextureRect().size.x,
        600.f / background.getTextureRect().size.y
    ));
    
    createSlots(slotTexture);

    centerBar.setPosition(sf::Vector2f(0.f, 300.f));

} 

void Field::createSlots(const sf::Texture& slotTexture) {
    //stabilisco le coordinate di partenza
    float topY = 100.f;
    float bottomY = 350.f;
    float xStart = 100.f;
    float spacing = 10.f; 

    // 3 mostri sopra, 3 magie sotto per ciascun giocatore

    //------- Giocatore 1 ------- 
   for (int i = 0; i < 3; ++i) {
        slots.emplace_back(sf::Vector2f(xStart + i * (slotWidth + spacing), bottomY), monsterTexture, Slot::Type::Monster);
        slots.emplace_back(sf::Vector2f(xStart + i * (slotWidth + spacing), bottomY + slotHeight + spacing), spellTexture, Slot::Type::SpellTrap);
    }

    slots.emplace_back(sf::Vector2f(600.f, bottomY), deckTexture, Slot::Type::Deck);
    slots.emplace_back(sf::Vector2f(600.f, bottomY + slotHeight + spacing), graveTexture, Slot::Type::Graveyard);
    slots.emplace_back(sf::Vector2f(680.f, bottomY), extraTexture, Slot::Type::Extra);
    slots.emplace_back(sf::Vector2f(680.f, bottomY + slotHeight + spacing), fieldSpellTexture, Slot::Type::FieldSpell);

    // ------- Giocatore 2 -------
    for (int i = 0; i < 3; ++i) {
        slots.emplace_back(sf::Vector2f(xStart + i * (slotWidth + spacing), topY), monsterTexture, Slot::Type::Monster);
        slots.emplace_back(sf::Vector2f(xStart + i * (slotWidth + spacing), topY + slotHeight + spacing), spellTexture, Slot::Type::SpellTrap);
    }

    slots.emplace_back(sf::Vector2f(600.f, topY), deckTexture, Slot::Type::Deck);
    slots.emplace_back(sf::Vector2f(600.f, topY + slotHeight + spacing), graveTexture, Slot::Type::Graveyard);
    slots.emplace_back(sf::Vector2f(680.f, topY), extraTexture, Slot::Type::Extra);
    slots.emplace_back(sf::Vector2f(680.f, topY + slotHeight + spacing), fieldSpellTexture, Slot::Type::FieldSpell);

}

void Field::draw(sf::RenderWindow& window, const sf::Vector2i& mousePos, const sf::Texture& centerBarTexture) const {
    window.draw(background);

    for (const auto& slot : slots) {
        bool hovered = slot.isHovered(mousePos);
        slot.draw(window, hovered);
    }
    
    window.draw(centerBar);

}


 