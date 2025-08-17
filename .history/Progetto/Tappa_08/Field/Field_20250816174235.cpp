#include "Field.h"
#include <cmath>
#include <iostream>

Field::Field(sf::Vector2u windowSize, sf::Vector2f slotSize, sf::Texture& slotTexture, sf::Texture& monsterTexture, sf::Texture& spellTrapTexture, sf::Texture& fieldSpellTexture, sf::Texture& deckTexture, sf::Texture& graveyardTexture, sf::Texture& extraDeckTexture): windowSize(windowSize), animationFinished(false), scaleFactor(1.f) {
    
    // Creo una renderTexture per ruotare la texture
    renderTexture.create(windowSize.x, windowSize.y);
    
    // Creo delle versioni ruotate delle texture per il giocatore 2
    createRotatedTexture(slotTexture, rotatedSlotTexture);
    createRotatedTexture(monsterTexture, rotatedMonsterTexture);
    createRotatedTexture(spellTrapTexture, rotatedSpellTrapTexture);
    createRotatedTexture(fieldSpellTexture, rotatedFieldSpellTexture);
    createRotatedTexture(deckTexture, rotatedDeckTexture);
    createRotatedTexture(graveyardTexture, rotatedGraveyardTexture);
    createRotatedTexture(extraDeckTexture, rotatedExtraDeckTexture);
    
    // Calcolo le posizioni per entrambi i giocatori
    float centerY = windowSize.y / 2.0f - windowSize.y * 0.05f;  // Leggermente più in alto del centro
    
    // Calcolo la spaziatura tra slot in base alla risoluzione
    float spacing = std::max(8.0f, windowSize.x * 0.005f);
    
    // Calcolo offset per le zone
    float zoneOffset = windowSize.y * 0.05f; // 5% dell'altezza della finestra
    
    // Posizioni Y per Player 1 (in basso)
    float p1MonsterY = centerY + zoneOffset;
    float p1SpellY = p1MonsterY + slotSize.y + spacing;
    
    // Posizioni Y per Player 2 (in alto)
    float p2MonsterY = centerY - zoneOffset;
    float p2SpellY = p2MonsterY - slotSize.y - spacing;
    
    // Slot dei mostri e magie/trappole per entrambi i giocatori (5 slot ciascuno)
    float startXMain = (windowSize.x - (5 * slotSize.x + 4 * spacing)) / 2.0f;
    
    // Slot principale del giocatore 1 (in basso)
    for (int i = 0; i < 5; ++i) {
        // Mostri
        slots.emplace_back(sf::Vector2f(startXMain + i * (slotSize.x + spacing), p1MonsterY), monsterTexture, Type::Monster, slotSize);
        // Magie/Trappole
        slots.emplace_back(sf::Vector2f(startXMain + i * (slotSize.x + spacing), p1SpellY), spellTrapTexture, Type::SpellTrap, slotSize);
    }
    
    // Slot principale del giocatore 2 (in alto)
    for (int i = 0; i < 5; ++i) {
        // Mostri
        slots.emplace_back(sf::Vector2f(startXMain + i * (slotSize.x + spacing), p2MonsterY), rotatedMonsterTexture, Type::Monster, slotSize);
        // Magie/Trappole
        slots.emplace_back(sf::Vector2f(startXMain + i * (slotSize.x + spacing), p2SpellY), rotatedSpellTrapTexture, Type::SpellTrap, slotSize);
    }
    
    // Calcolo margini per le zone speciali
    float margin = std::max(20.0f, windowSize.x * 0.02f);
    float leftX = margin;
    float rightX = windowSize.x - slotSize.x - margin;
    
    // Zone speciali giocatore 1 (destra del campo)
    slots.emplace_back(sf::Vector2f(rightX, p1MonsterY), graveyardTexture, Type::Graveyard, slotSize);
    slots.emplace_back(sf::Vector2f(rightX, p1SpellY), deckTexture, Type::Deck, slotSize);
    
    // Zone speciali giocatore 2 (sinistra del campo)
    slots.emplace_back(sf::Vector2f(leftX, p2MonsterY), rotatedGraveyardTexture, Type::Graveyard, slotSize);
    slots.emplace_back(sf::Vector2f(leftX, p2SpellY), rotatedDeckTexture, Type::Deck, slotSize);
    
    // Slot extra deck e field spell
    slots.emplace_back(sf::Vector2f(leftX, p1SpellY), extraDeckTexture, Type::Extra, slotSize);
    slots.emplace_back(sf::Vector2f(rightX, p2SpellY), rotatedExtraDeckTexture, Type::Extra, slotSize);
    
    // Slot field spell centrale
    float fieldSpellX = windowSize.x / 2.0f - slotSize.x / 2.0f;
    float fieldSpellY = centerY - slotSize.y / 2.0f;
    slots.emplace_back(sf::Vector2f(fieldSpellX, fieldSpellY), fieldSpellTexture, Type::FieldSpell, slotSize);
}

void Field::animate(float deltaTime) {
    if (!animationFinished) {
        // Animazione di zoom/fade-in degli slot
        const float animationSpeed = 2.0f; // velocità dell'animazione
        const float maxScale = 1.0f;
        
        scaleFactor += animationSpeed * deltaTime;
        
        if (scaleFactor >= maxScale) {
            scaleFactor = maxScale;
            animationFinished = true;
        }
        
        // Applica la scala a tutti gli slot
        for (auto& slot : slots) {
            slot.setScale(scaleFactor);
        }
    }
}

bool Field::isAnimationFinished() const{
    return animationFinished; 
}

void Field::draw(sf::RenderWindow& window, const sf::Vector2i& mousePos, GameState gamestate) {
    for (auto& slot : slots) {
        slot.draw(window, mousePos, gamestate);
    }
}

void Field::createRotatedTexture(const sf::Texture& original, sf::Texture& rotated) {
    // Pulisco la renderTexture
    renderTexture.clear(sf::Color::Transparent);
    
    // Creo uno sprite con la texture originale
    sf::Sprite sprite(original);
    
    // Imposto l'origine al centro
    sf::Vector2u size = original.getSize();
    sprite.setOrigin(size.x / 2.0f, size.y / 2.0f);
    
    // Posiziono al centro della renderTexture
    sprite.setPosition(windowSize.x / 2.0f, windowSize.y / 2.0f);
    
    // Ruoto di 180 gradi
    sprite.setRotation(180.0f);
    
    // Disegno sulla renderTexture
    renderTexture.draw(sprite);
    renderTexture.display();
    
    // Copia il risultato nella texture di destinazione
    rotated = renderTexture.getTexture();
}

sf::Vector2f Field::getSlotPosition(Type slotType, int player, int slotIndex) const {
    std::cout << "DEBUG: Cercando slot di tipo " << static_cast<int>(slotType) 
              << " per giocatore " << player << " con indice " << slotIndex << std::endl;
    
    // Conta gli slot per tipo e organizza per giocatore
    std::vector<sf::Vector2f> player1Slots;
    std::vector<sf::Vector2f> player2Slots;
    
    // Raccogli tutte le posizioni per il tipo di slot richiesto
    for (const auto& slot : slots) {
        if (slot.getType() == slotType) {
            sf::Vector2f pos = slot.getPosition();
            
            // Determina a quale giocatore appartiene lo slot basandosi sulla posizione Y
            // Gli slot del giocatore 1 sono nella metà inferiore del campo
            if (pos.y > windowSize.y / 2.0f) {
                player1Slots.push_back(pos);
            } else {
                player2Slots.push_back(pos);
            }
        }
    }
    
    std::cout << "DEBUG: Trovati " << player1Slots.size() << " slot per P1 e " 
              << player2Slots.size() << " slot per P2" << std::endl;
    
    // Seleziona il vettore corretto in base al giocatore
    const std::vector<sf::Vector2f>& targetSlots = (player == 1) ? player1Slots : player2Slots;
    
    // Verifica che l'indice sia valido
    if (slotIndex >= 0 && slotIndex < static_cast<int>(targetSlots.size())) {
        std::cout << "DEBUG: Restituisco posizione " << targetSlots[slotIndex].x 
                  << ", " << targetSlots[slotIndex].y << std::endl;
        return targetSlots[slotIndex];
    }
    
    std::cout << "DEBUG: Indice non valido, restituisco (0,0)" << std::endl;
    // Se l'indice non è valido, restituisce (0,0)
    return sf::Vector2f(0.f, 0.f);
}

Slot* Field::getSlotByPosition(sf::Vector2f position, float tolerance) {
    // Aumentiamo la tolleranza di default se non specificata
    if (tolerance <= 0.0f) {
        tolerance = 75.0f; // Aumentato da 50.0f a 75.0f
    }
    
    // Cerca lo slot con la posizione più vicina alla posizione data
    for (auto& slot : slots) {
        sf::Vector2f slotPos = slot.getPosition();
        
        // Calcola la distanza tra le posizioni
        float distance = std::sqrt(std::pow(slotPos.x - position.x, 2) + std::pow(slotPos.y - position.y, 2));
        
        // Se la distanza è entro la tolleranza, restituisce questo slot
        if (distance <= tolerance) {
            return &slot;
        }
    }
    
    // Se non trova nessuno slot corrispondente, restituisce nullptr
    return nullptr;
}

void Field::setAnimationFinished() {
    animationFinished = true; 
}

bool Field::isAnimationFinished() const{
    return animationFinished; 
}