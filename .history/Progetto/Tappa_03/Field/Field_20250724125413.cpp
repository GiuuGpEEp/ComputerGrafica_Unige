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
    // Calcola offset dinamico per evitare overflow e lasciare spazio per le carte
    float maxTotalHeight = 4 * slotSize.y + 3 * spacing; // 2 righe per giocatore + spaziature
    float zoneOffset = std::min(windowSize.y * 0.06f, (windowSize.y - maxTotalHeight) / 5.0f); // Ridotto per più margine
    
    //------- GIOCATORE 1 (in basso) -------
    float p1MonsterY = centerY + zoneOffset;
    float p1SpellY = p1MonsterY + slotSize.y + spacing;
    
    // 3 slot mostri giocatore 1
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p1MonsterY), monsterTexture, Slot::Type::Monster, slotSize);
    }
    
    // 3 slot magie/trappole giocatore 1
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p1SpellY), spellTrapTexture, Slot::Type::SpellTrap, slotSize);
    }

    //------- GIOCATORE 2 (in alto) -------
    float p2SpellY = centerY - zoneOffset - slotSize.y;
    float p2MonsterY = p2SpellY - slotSize.y - spacing;
    
    // Crea la texture ruotata per le magie del giocatore 2
    createRotatedTexture(monsterTexture, rotatedMonsterTexture);

    // 3 slot magie/trappole giocatore 2
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p2SpellY), rotatedMonsterTexture, Slot::Type::Monster, slotSize);
    }
    
    // Crea la texture ruotata per le magie del giocatore 2
    createRotatedTexture(spellTrapTexture, rotatedSpellTrapTexture);

    // 3 slot mostri giocatore 2 (con texture ruotata)
    for (int i = 0; i < 3; ++i) {
        float x = mainStartX + i * (slotSize.x + spacing);
        slots.emplace_back(sf::Vector2f(x, p2MonsterY), spellTrapTexture, Slot::Type::SpellTrap, slotSize);
    }

    //------- ZONE SPECIALI -------
    // Margine dinamico dai bordi - aumentato per più spazio per le carte
    float margin = std::max(windowSize.x * 0.04f, 20.0f); // Aumentato a 4% della larghezza o min 20px
    float rightX = windowSize.x - slotSize.x - margin;
    float leftX = margin;
    
    // Verifica che le zone speciali non si sovrappongano alle zone principali
    float mainZoneRight = mainStartX + mainZoneWidth;
    if (rightX < mainZoneRight + spacing * 2) { // Aumentato spacing per più distanza
        rightX = std::min(windowSize.x - slotSize.x - 10.0f, mainZoneRight + spacing * 2);
    }
    if (leftX + slotSize.x + spacing * 2 > mainStartX) { // Aumentato spacing per più distanza
        leftX = std::max(10.0f, mainStartX - slotSize.x - spacing * 2);
    }
    
    // Zone speciali giocatore 1 (destra del campo)
    slots.emplace_back(sf::Vector2f(rightX, p1MonsterY), graveyardTexture, Slot::Type::Graveyard, slotSize);
    slots.emplace_back(sf::Vector2f(rightX, p1SpellY), deckTexture, Slot::Type::Deck, slotSize);

    // Crea la texture ruotata per le Extra e FieldSpell del giocatore 2
    createRotatedTexture(extraDeckTexture, rotatedExtraDeckTexture);
    createRotatedTexture(fieldSpellTexture, rotatedFieldSpellTexture);

    // Zone speciali giocatore 2 (destra del campo)
    slots.emplace_back(sf::Vector2f(rightX, p2MonsterY), rotatedExtraDeckTexture, Slot::Type::Extra, slotSize);
    slots.emplace_back(sf::Vector2f(rightX, p2SpellY), rotatedFieldSpellTexture, Slot::Type::FieldSpell, slotSize);

    // Extra Deck e Field Spell a sinistra
    slots.emplace_back(sf::Vector2f(leftX, p1MonsterY), fieldSpellTexture, Slot::Type::FieldSpell, slotSize);
    slots.emplace_back(sf::Vector2f(leftX, p1SpellY), extraDeckTexture, Slot::Type::Extra, slotSize);

    slots.emplace_back(sf::Vector2f(leftX, p2MonsterY), fieldSpellTexture, Slot::Type::Extra, slotSize);
    slots.emplace_back(sf::Vector2f(leftX, p2SpellY), extraDeckTexture, Slot::Type::FieldSpell, slotSize);
}

sf::Vector2f Field::calculateSlotSize() const {
    // Calcola dimensioni slot in base alla finestra
    // Aumentato ulteriormente la percentuale per slot ancora più grandi
    float availableWidth = windowSize.x * 0.72f; // Aumentato da 68% a 72%
    float slotWidth = availableWidth / 5.0f; // 3 slot + 2 spaziature equivalenti
    
    // Mantieni proporzioni 128:180 dalla texture originale  
    float aspectRatio = 180.0f / 128.0f;
    float slotHeight = slotWidth * aspectRatio;
    
    // Aumentato ancora l'altezza massima
    float maxHeight = windowSize.y * 0.16f; // Aumentato da 14% a 16%
    if (slotHeight > maxHeight) {
        slotHeight = maxHeight;
        slotWidth = slotHeight / aspectRatio;
    }
    
    // Aumentato anche la dimensione minima
    float minHeight = 70.0f; // Aumentato da 60 a 70
    if (slotHeight < minHeight) {
        slotHeight = minHeight;
        slotWidth = slotHeight / aspectRatio;
    }
    
    return sf::Vector2f(slotWidth, slotHeight);
}

float Field::calculateSpacing() const {
    // Spaziatura proporzionale alla dimensione della finestra
    return windowSize.x * 0.01f; // 1% della larghezza finestra
} 

void Field::draw(sf::RenderWindow& window, const sf::Vector2i& mousePos) const {
    window.draw(background);

    for (const auto& slot : slots) {
        bool hovered = slot.isHovered(mousePos);
        slot.draw(window, hovered);
    }
}

void Field::createRotatedTexture(const sf::Texture& original, sf::Texture& rotated) {
    
    /*Le texture sono solo immagini, non possono essere ruotate, quindi creo uno sprite a cui applico 
    la texture e successivamente posso ruotarla. Per poi applicarla a un'altra immagine sfrutto una
    RenderTexture su cui disegno la texture ruotata per poi applicarla su un'altra texture. 
    Mentre le texture normali sono solo delle immagini da caricare, le renderTexture sono delle 
    texture speciali su cui si può disegnare, ciò permette di applicare trasformazioni 
    ad un immagine per poi riusarla come texture*/
    
    sf::RenderTexture renderTexture;  
    
    // Ottieni le dimensioni della texture originale
    sf::Vector2u size = original.getSize();
    
    // Crea un RenderTexture della stessa dimensione
    renderTexture.resize(size);
    
    // Crea uno sprite dalla texture originale
    sf::Sprite sprite(original);
    
    // Imposta l'origine al centro dello sprite
    sprite.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
    
    // Posiziona lo sprite al centro del RenderTexture
    sprite.setPosition(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
    
    // Ruota di 180 gradi
    sprite.setRotation(sf::degrees(180.0f));
    
    // Renderizza lo sprite ruotato
    renderTexture.clear(sf::Color::Transparent);
    renderTexture.draw(sprite);
    renderTexture.display();
    
    // Copia il risultato nella texture di destinazione
    rotated = renderTexture.getTexture();
}


 