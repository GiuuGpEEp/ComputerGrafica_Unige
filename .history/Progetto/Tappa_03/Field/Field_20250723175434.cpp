#include "Field.h"

Field::Field(
            const sf::Texture& fieldTexture, 
            const sf::Texture& monsterTexture,
            const sf::Texture& spellTrapTexture,
            const sf::Texture& deckTexture,
            const sf::Texture& graveyardTexture,
            const sf::Texture& extraDeckTexture,
            const sf::Texture& fieldSpellTexture,
            const sf::Vector2u& windowSize
        ) : background(fieldTexture), windowSize(windowSize)
{
    // Scala il background per coprire l'intera finestra
    sf::Vector2u textureSize = fieldTexture.getSize();
    float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(textureSize.x);
    float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(textureSize.y);
    background.setScale(sf::Vector2f(scaleX, scaleY));
    
    slots.reserve(16);

    // Calcola dimensioni e spaziatura dinamiche
    sf::Vector2f slotSize = calculateSlotSize();
    float spacing = calculateSpacing();
    
    // Centro della finestra
    float centerX = windowSize.x / 2.0f;
    float centerY = windowSize.y / 2.0f;
    
    // Calcolo posizioni per le zone principali (3 mostri + 3 magie/trappole)
    float mainZoneWidth = 3 * slotSize.x + 2 * spacing;
    float mainStartX = centerX - mainZoneWidth / 2.0f;
    
    // Distanza dal centro per le zone principali (proporzionale all'altezza)
    float zoneOffset = windowSize.y * 0.12f; // 12% dell'altezza finestra
    
    //------- GIOCATORE 1 (in basso) -------
    float p1MonsterY = centerY + zoneOffset;
    float p1SpellY = p1MonsterY + slotSize.y + spacing;
    
    // 3 slot mostri giocatore 1
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p1MonsterY), slotTexture, Slot::Type::Monster, slotSize);
    }
    
    // 3 slot magie/trappole giocatore 1
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p1SpellY), slotTexture, Slot::Type::SpellTrap, slotSize);
    }

    //------- GIOCATORE 2 (in alto) -------
    float p2SpellY = centerY - zoneOffset - slotSize.y;
    float p2MonsterY = p2SpellY - slotSize.y - spacing;
    
    // 3 slot magie/trappole giocatore 2 
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p2SpellY), slotTexture, Slot::Type::SpellTrap, slotSize);
    }
    
    // 3 slot mostri giocatore 2
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p2MonsterY), slotTexture, Slot::Type::Monster, slotSize);
    }

    //------- ZONE SPECIALI -------
    // Margine dinamico dai bordi
    float margin = windowSize.x * 0.05f; // 5% della larghezza
    float rightX = windowSize.x - slotSize.x - margin;
    float leftX = margin;
    
    // Zone speciali giocatore 1 (destra del campo)
    slots.emplace_back(sf::Vector2f(rightX, p1MonsterY), slotTexture, Slot::Type::Deck, slotSize);
    slots.emplace_back(sf::Vector2f(rightX, p1SpellY), slotTexture, Slot::Type::Graveyard, slotSize);
    
    // Zone speciali giocatore 2 (destra del campo)
    slots.emplace_back(sf::Vector2f(rightX, p2MonsterY), slotTexture, Slot::Type::Deck, slotSize);
    slots.emplace_back(sf::Vector2f(rightX, p2SpellY), slotTexture, Slot::Type::Graveyard, slotSize);
    
    // Extra Deck e Field Spell a sinistra
    slots.emplace_back(sf::Vector2f(leftX, p1MonsterY), slotTexture, Slot::Type::Extra, slotSize);
    slots.emplace_back(sf::Vector2f(leftX, p1SpellY), slotTexture, Slot::Type::FieldSpell, slotSize);
    
    slots.emplace_back(sf::Vector2f(leftX, p2MonsterY), slotTexture, Slot::Type::Extra, slotSize);
    slots.emplace_back(sf::Vector2f(leftX, p2SpellY), slotTexture, Slot::Type::FieldSpell, slotSize);
}
}

sf::Vector2f Field::calculateSlotSize() const {
    // Calcola dimensioni slot in base alla finestra
    // Riserviamo circa 70% della larghezza per le 3 zone principali + margini
    float availableWidth = windowSize.x * 0.7f;
    float slotWidth = availableWidth / 5.0f; // 3 slot + 2 spaziature equivalenti
    
    // Mantieni proporzioni 128:180 dalla texture originale  
    float aspectRatio = 180.0f / 128.0f;
    float slotHeight = slotWidth * aspectRatio;
    
    // Limita l'altezza se troppo grande
    float maxHeight = windowSize.y * 0.18f; // Max 18% dell'altezza finestra
    if (slotHeight > maxHeight) {
        slotHeight = maxHeight;
        slotWidth = slotHeight / aspectRatio;
    }
    
    return sf::Vector2f(slotWidth, slotHeight);
}

float Field::calculateSpacing() const {
    // Spaziatura proporzionale alla dimensione della finestra
    return windowSize.x * 0.01f; // 1% della larghezza finestra
} 

void Field::createSlots(const sf::Texture& slotTexture) {
    slots.reserve(16);

    // Calcola dimensioni e spaziatura dinamiche
    sf::Vector2f slotSize = calculateSlotSize();
    float spacing = calculateSpacing();
    
    // Centro della finestra
    float centerX = windowSize.x / 2.0f;
    float centerY = windowSize.y / 2.0f;
    
    // Calcolo posizioni per le zone principali (3 mostri + 3 magie/trappole)
    float mainZoneWidth = 3 * slotSize.x + 2 * spacing;
    float mainStartX = centerX - mainZoneWidth / 2.0f;
    
    // Distanza dal centro per le zone principali (proporzionale all'altezza)
    float zoneOffset = windowSize.y * 0.12f; // 12% dell'altezza finestra
    
    //------- GIOCATORE 1 (in basso) -------
    float p1MonsterY = centerY + zoneOffset;
    float p1SpellY = p1MonsterY + slotSize.y + spacing;
    
    // 3 slot mostri giocatore 1
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p1MonsterY), slotTexture, Slot::Type::Monster, slotSize);
    }
    
    // 3 slot magie/trappole giocatore 1
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p1SpellY), slotTexture, Slot::Type::SpellTrap, slotSize);
    }

    //------- GIOCATORE 2 (in alto) -------
    float p2SpellY = centerY - zoneOffset - slotSize.y;
    float p2MonsterY = p2SpellY - slotSize.y - spacing;
    
    // 3 slot magie/trappole giocatore 2 
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p2SpellY), slotTexture, Slot::Type::SpellTrap, slotSize);
    }
    
    // 3 slot mostri giocatore 2
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p2MonsterY), slotTexture, Slot::Type::Monster, slotSize);
    }

    //------- ZONE SPECIALI -------
    // Margine dinamico dai bordi
    float margin = windowSize.x * 0.05f; // 5% della larghezza
    float rightX = windowSize.x - slotSize.x - margin;
    float leftX = margin;
    
    // Zone speciali giocatore 1 (destra del campo)
    slots.emplace_back(sf::Vector2f(rightX, p1MonsterY), slotTexture, Slot::Type::Deck, slotSize);
    slots.emplace_back(sf::Vector2f(rightX, p1SpellY), slotTexture, Slot::Type::Graveyard, slotSize);
    
    // Zone speciali giocatore 2 (destra del campo)
    slots.emplace_back(sf::Vector2f(rightX, p2MonsterY), slotTexture, Slot::Type::Deck, slotSize);
    slots.emplace_back(sf::Vector2f(rightX, p2SpellY), slotTexture, Slot::Type::Graveyard, slotSize);
    
    // Extra Deck e Field Spell a sinistra
    slots.emplace_back(sf::Vector2f(leftX, p1MonsterY), slotTexture, Slot::Type::Extra, slotSize);
    slots.emplace_back(sf::Vector2f(leftX, p1SpellY), slotTexture, Slot::Type::FieldSpell, slotSize);
    
    slots.emplace_back(sf::Vector2f(leftX, p2MonsterY), slotTexture, Slot::Type::Extra, slotSize);
    slots.emplace_back(sf::Vector2f(leftX, p2SpellY), slotTexture, Slot::Type::FieldSpell, slotSize);
}

void Field::draw(sf::RenderWindow& window, const sf::Vector2i& mousePos) const {
    window.draw(background);

    for (const auto& slot : slots) {
        bool hovered = slot.isHovered(mousePos);
        slot.draw(window, hovered);
    }

}


 