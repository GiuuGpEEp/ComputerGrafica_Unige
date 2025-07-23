#pragma once
#include <SFML/Graphics.hpp>
#include "../Slot/Slot.h"
#include <vector>

class Field {
public:
    Field(
        const sf::Texture& fieldTexture, 
        const sf::Texture& monsterTexture,
        const sf::Texture& spellTrapTexture,
        const sf::Texture& deckTexture,

        const sf::Vector2u& windowSize  // Aggiungiamo la dimensione della finestra
    );
    void draw(sf::RenderWindow& window, const sf::Vector2i& mousePos) const;

private:
    sf::Sprite background;
    std::vector<Slot> slots;
    sf::Vector2u windowSize;  // Memorizza la dimensione della finestra
    
    void createSlots(const sf::Texture& slotTexture);
    sf::Vector2f calculateSlotSize() const;  // Calcola dimensioni ottimali slot
    float calculateSpacing() const;          // Calcola spaziatura ottimale
};
