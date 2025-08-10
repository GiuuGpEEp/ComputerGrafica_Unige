#pragma once
#include <SFML/Graphics.hpp>
#include "../Slot/Slot.h"
#include "../auxFunc.h"
#include <vector>

class Field {
public:
    Field(
        const sf::Texture& fieldTexture, 
        const sf::Texture& monsterTexture,
        const sf::Texture& spellTrapTexture,
        const sf::Texture& deckTexture,
        const sf::Texture& graveyardTexture,
        const sf::Texture& extraDeckTexture,
        const sf::Texture& fieldSpellTexture,
        const sf::Vector2u& windowSize  // Aggiungiamo la dimensione della finestra
    );
    void draw(sf::RenderWindow& window, const sf::Vector2i& mousePos, GameState gamestate) const;
    
    // Metodo per ottenere la posizione di uno slot per tipo, giocatore e indice slot
    sf::Vector2f getSlotPosition(Type slotType, int player = 1, int slotIndex = 0) const;
    
    // Metodo per ottenere lo slot dalla sua posizione
    Slot* getSlotByPosition(sf::Vector2f position, float tolerance = 1.0f);
    
    void animate(float deltaTime);
    bool isAnimationFinished() const;
    void setAnimationFinished();

private:
    sf::Sprite background;
    std::vector<Slot> slots;
    sf::Vector2u windowSize;  // Memorizza la dimensione della finestra
    bool animationFinished = false; 
    float fieldAlpha = 0.f; 
    float fieldOffset = 200.f; 
    
    // Texture ruotate per il giocatore 2
    sf::Texture rotatedMonsterTexture;
    sf::Texture rotatedSpellTrapTexture;
    sf::Texture rotatedDeckTexture;
    sf::Texture rotatedGraveyardTexture;
    sf::Texture rotatedExtraDeckTexture;
    sf::Texture rotatedFieldSpellTexture;

    void createRotatedTexture(const sf::Texture& original, sf::Texture& rotated); // Crea texture ruotata
};
