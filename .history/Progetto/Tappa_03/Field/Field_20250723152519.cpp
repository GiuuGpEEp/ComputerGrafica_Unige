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
    slots.reserve(16); // Aumentato per ospitare tutti gli slot

    // Dimensioni degli slot (come definito in Slot.cpp)
    const float slotWidth = 128.f;
    const float slotHeight = 180.f;
    const float spacing = 10.f; // Spaziatura tra slot
    
    // Posizioni di base per il layout
    const float centerX = 400.f; // Centro orizzontale della finestra (800/2)
    const float fieldCenterY = 300.f; // Centro verticale della finestra (600/2)
    
    // Calcolo posizioni per un layout simmetrico
    const float totalWidth = 3 * slotWidth + 2 * spacing; // Larghezza totale per 3 slot
    const float startX = centerX - totalWidth / 2.f; // Punto di partenza X centrato
    
    // Posizioni Y per i due giocatori
    const float player1MonsterY = fieldCenterY + 60.f;  // Mostri giocatore 1 (in basso)
    const float player1SpellY = player1MonsterY + slotHeight + spacing; // Magie giocatore 1
    
    const float player2MonsterY = fieldCenterY - 60.f - slotHeight; // Mostri giocatore 2 (in alto)
    const float player2SpellY = player2MonsterY - slotHeight - spacing; // Magie giocatore 2
    
    //------- Giocatore 1 (in basso) ------- 
    // 3 slot mostri
    for (int i = 0; i < 3; ++i) {
        float x = startX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, player1MonsterY), slotTexture, Slot::Type::Monster);
    }
    
    // 3 slot magie/trappole
    for (int i = 0; i < 3; ++i) {
        float x = startX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, player1SpellY), slotTexture, Slot::Type::SpellTrap);
    }

    // Slot speciali giocatore 1 (a destra)
    float specialX = startX + totalWidth + 50.f;
    slots.emplace_back(sf::Vector2f(specialX, player1MonsterY), slotTexture, Slot::Type::Deck);
    slots.emplace_back(sf::Vector2f(specialX, player1SpellY), slotTexture, Slot::Type::Graveyard);
    slots.emplace_back(sf::Vector2f(specialX + slotWidth + spacing, player1MonsterY), slotTexture, Slot::Type::Extra);
    slots.emplace_back(sf::Vector2f(specialX + slotWidth + spacing, player1SpellY), slotTexture, Slot::Type::FieldSpell);

    // ------- Giocatore 2 (in alto) -------
    // 3 slot mostri
    for (int i = 0; i < 3; ++i) {
        float x = startX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, player2MonsterY), slotTexture, Slot::Type::Monster);
    }
    
    // 3 slot magie/trappole  
    for (int i = 0; i < 3; ++i) {
        float x = startX + i * (slotWidth + spacing);
        slots.emplace_back(sf::Vector2f(x, player2SpellY), slotTexture, Slot::Type::SpellTrap);
    }

    // Slot speciali giocatore 2 (a destra)
    slots.emplace_back(sf::Vector2f(specialX, player2MonsterY), slotTexture, Slot::Type::Deck);
    slots.emplace_back(sf::Vector2f(specialX, player2SpellY), slotTexture, Slot::Type::Graveyard);
    slots.emplace_back(sf::Vector2f(specialX + slotWidth + spacing, player2MonsterY), slotTexture, Slot::Type::Extra);
    slots.emplace_back(sf::Vector2f(specialX + slotWidth + spacing, player2SpellY), slotTexture, Slot::Type::FieldSpell);
}

void Field::draw(sf::RenderWindow& window, const sf::Vector2i& mousePos) const {
    window.draw(background);

    for (const auto& slot : slots) {
        bool hovered = slot.isHovered(mousePos);
        slot.draw(window, hovered);
    }

}


 