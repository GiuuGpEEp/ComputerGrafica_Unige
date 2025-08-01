#include "Field.h"

Field::Field(
            const sf::Texture& fieldTexture, 
            const sf::Texture& slotTexture
        ) : background(fieldTexture)
{
    // Scala il background per coprire l'intera finestra (800x600)
    sf::Vector2u textureSize = fieldTexture.getSize();
    float scaleX = 800.f / static_cast<float>(textureSize.x);
    float scaleY = 600.f / static_cast<float>(textureSize.y);
    background.setScale(sf::Vector2f(scaleX, scaleY));
    
    createSlots(slotTexture);
} 

void Field::createSlots(const sf::Texture& slotTexture) {
    slots.reserve(16); // 6 zone principali + 4 zone speciali per ogni giocatore

    // Dimensioni ridotte degli slot per adattarsi meglio alla finestra
    const float slotWidth = 90.f;   // Ridotto da 128 a 90
    const float slotHeight = 130.f; // Ridotto da 180 a 130
    const float spacing = 8.f;      // Spaziatura ridotta
    
    // Centro della finestra
    const float centerX = 400.f; // Centro orizzontale (800/2)
    const float centerY = 300.f; // Centro verticale (600/2)
    
    // Calcolo posizioni per le zone principali (3 mostri + 3 magie/trappole)
    const float mainZoneWidth = 3 * slotWidth + 2 * spacing;
    const float mainStartX = centerX - mainZoneWidth / 2.f;
    
    // Distanza dal centro per le zone principali
    const float zoneOffset = 65.f; // Ridotto per evitare overflow
    
    //------- GIOCATORE 1 (in basso) -------
    const float p1MonsterY = centerY + zoneOffset;
    const float p1SpellY = p1MonsterY + slotHeight + spacing;
    
    // 3 slot mostri giocatore 1
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, p1MonsterY), slotTexture, Slot::Type::Monster);
    }
    
    // 3 slot magie/trappole giocatore 1
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, p1SpellY), slotTexture, Slot::Type::SpellTrap);
    }

    //------- GIOCATORE 2 (in alto) -------
    const float p2SpellY = centerY - zoneOffset - slotHeight;
    const float p2MonsterY = p2SpellY - slotHeight - spacing;
    
    // 3 slot magie/trappole giocatore 2 
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, p2SpellY), slotTexture, Slot::Type::SpellTrap);
    }
    
    // 3 slot mostri giocatore 2
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, p2MonsterY), slotTexture, Slot::Type::Monster);
    }

    //------- ZONE SPECIALI -------
    // Posizioni per zone speciali a destra e sinistra del campo principale
    const float rightX = mainStartX + mainZoneWidth + 25.f;
    const float leftX = mainStartX - slotWidth - 25.f;
    
    // Zone speciali giocatore 1 (destra del campo)
    slots.emplace_back(sf::Vector2f(rightX, p1MonsterY), slotTexture, Slot::Type::Deck);
    slots.emplace_back(sf::Vector2f(rightX, p1SpellY), slotTexture, Slot::Type::Graveyard);
    
    // Zone speciali giocatore 2 (destra del campo)
    slots.emplace_back(sf::Vector2f(rightX, p2MonsterY), slotTexture, Slot::Type::Deck);
    slots.emplace_back(sf::Vector2f(rightX, p2SpellY), slotTexture, Slot::Type::Graveyard);
    
    // Extra Deck e Field Spell a sinistra
    slots.emplace_back(sf::Vector2f(leftX, p1MonsterY), slotTexture, Slot::Type::Extra);
    slots.emplace_back(sf::Vector2f(leftX, p1SpellY), slotTexture, Slot::Type::FieldSpell);
    
    slots.emplace_back(sf::Vector2f(leftX, p2MonsterY), slotTexture, Slot::Type::Extra);
    slots.emplace_back(sf::Vector2f(leftX, p2SpellY), slotTexture, Slot::Type::FieldSpell);
}

void Field::draw(sf::RenderWindow& window, const sf::Vector2i& mousePos) const {
    window.draw(background);

    for (const auto& slot : slots) {
        bool hovered = slot.isHovered(mousePos);
        slot.draw(window, hovered);
    }

}


 