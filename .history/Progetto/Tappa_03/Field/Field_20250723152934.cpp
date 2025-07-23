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
    slots.reserve(20); // Aumentato per ospitare tutti gli slot

    // Dimensioni degli slot (come definito nella texture)
    const float slotWidth = 128.f;
    const float slotHeight = 180.f;
    const float spacing = 15.f; // Spaziatura tra slot
    
    // Layout simile a Yu-Gi-Oh!
    // Centro della finestra
    const float centerX = 400.f; // Centro orizzontale (800/2)
    const float centerY = 300.f; // Centro verticale (600/2)
    
    // Calcolo posizioni per zone principali (5 slot monster + 5 slot spell/trap per giocatore)
    const float mainZoneWidth = 5 * slotWidth + 4 * spacing;
    const float mainStartX = centerX - mainZoneWidth / 2.f;
    
    // Distanza dal centro per le zone principali
    const float zoneOffset = 95.f;
    
    //------- GIOCATORE 1 (in basso) -------
    const float p1MonsterY = centerY + zoneOffset;
    const float p1SpellY = p1MonsterY + slotHeight + spacing;
    
    // 5 slot mostri giocatore 1
    for (int i = 0; i < 5; ++i) {
        float x = mainStartX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, p1MonsterY), slotTexture, Slot::Type::Monster);
    }
    
    // 5 slot magie/trappole giocatore 1
    for (int i = 0; i < 5; ++i) {
        float x = mainStartX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, p1SpellY), slotTexture, Slot::Type::SpellTrap);
    }

    //------- GIOCATORE 2 (in alto) -------
    const float p2SpellY = centerY - zoneOffset - slotHeight;
    const float p2MonsterY = p2SpellY - slotHeight - spacing;
    
    // 5 slot magie/trappole giocatore 2 
    for (int i = 0; i < 5; ++i) {
        float x = mainStartX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, p2SpellY), slotTexture, Slot::Type::SpellTrap);
    }
    
    // 5 slot mostri giocatore 2
    for (int i = 0; i < 5; ++i) {
        float x = mainStartX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, p2MonsterY), slotTexture, Slot::Type::Monster);
    }

    //------- ZONE SPECIALI -------
    // Posizioni per zone deck/extra/graveyard
    const float specialX = mainStartX + mainZoneWidth + 30.f;
    const float fieldSpellX = mainStartX - slotWidth - 30.f;
    
    // Zone speciali giocatore 1 (destra)
    slots.emplace_back(sf::Vector2f(specialX, p1MonsterY), slotTexture, Slot::Type::Deck);
    slots.emplace_back(sf::Vector2f(specialX, p1SpellY), slotTexture, Slot::Type::Graveyard);
    slots.emplace_back(sf::Vector2f(specialX + slotWidth + spacing, p1MonsterY), slotTexture, Slot::Type::Extra);
    
    // Zone speciali giocatore 2 (destra)
    slots.emplace_back(sf::Vector2f(specialX, p2MonsterY), slotTexture, Slot::Type::Deck);
    slots.emplace_back(sf::Vector2f(specialX, p2SpellY), slotTexture, Slot::Type::Graveyard);
    slots.emplace_back(sf::Vector2f(specialX + slotWidth + spacing, p2MonsterY), slotTexture, Slot::Type::Extra);
    
    // Field Spell (centro-sinistra per entrambi)
    slots.emplace_back(sf::Vector2f(fieldSpellX, p1MonsterY + slotHeight/2.f), slotTexture, Slot::Type::FieldSpell);
    slots.emplace_back(sf::Vector2f(fieldSpellX, p2MonsterY + slotHeight/2.f), slotTexture, Slot::Type::FieldSpell);
}

void Field::draw(sf::RenderWindow& window, const sf::Vector2i& mousePos) const {
    window.draw(background);

    for (const auto& slot : slots) {
        bool hovered = slot.isHovered(mousePos);
        slot.draw(window, hovered);
    }

}


 