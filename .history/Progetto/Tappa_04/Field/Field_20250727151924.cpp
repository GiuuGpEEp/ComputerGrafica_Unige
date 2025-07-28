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
    // Trasformazioni per effetto prospettico
    sf::Transform p1Transf; // Giocatore 1 (in primo piano)
    p1Transf.scale(sf::Vector2f(1.0f, 1.0f)); // Dimensione normale

    sf::Transform p2Transf; // Giocatore 2 (in prospettiva)
    p2Transf.scale(sf::Vector2f(0.7f, 0.5f)); // Più piccolo per effetto profondità
    
    // Trasformazione prospettica: inclinazione verso l'alto
    // Usando una matrice di trasformazione per la prospettiva
    float perspectiveSkew = 0.15f; // Fattore di inclinazione prospettica
    sf::Transform perspectiveTransform(
        1.0f, perspectiveSkew, 0.0f,    // Riga 1: [1, skew, 0]
        0.0f, 1.0f, 0.0f,               // Riga 2: [0, 1, 0] 
        0.0f, 0.0f, 1.0f                // Riga 3: [0, 0, 1]
    );
    
    p2Transf = p2Transf * perspectiveTransform;

    // Scala il background per coprire l'intera finestra
    sf::Vector2u textureSize = fieldTexture.getSize();
    float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(textureSize.x);
    float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(textureSize.y);
    background.setScale(sf::Vector2f(scaleX, scaleY));
    
    slots.reserve(16);

    // Calcola dimensioni per la prospettiva
    sf::Vector2f slotSizeP1 = calculateSlotSize(); // Dimensione normale per P1
    sf::Vector2f slotSizeP2 = sf::Vector2f(slotSizeP1.x * 0.7f, slotSizeP1.y * 0.5f); // Più piccolo per P2
    
    float spacingP1 = calculateSpacing(); // Spacing normale per P1  
    float spacingP2 = spacingP1 * 0.6f; // Spacing ridotto per P2 per effetto prospettico
    
    // Centro della finestra
    float centerX = windowSize.x / 2.0f;
    float centerY = windowSize.y / 2.0f;
    
    // Layout prospettico: P1 occupa più spazio in basso, P2 compresso in alto
    float p1ZoneHeight = windowSize.y * 0.45f; // P1 occupa 45% della parte bassa
    float p2ZoneHeight = windowSize.y * 0.25f; // P2 occupa 25% della parte alta
    
    // Posizioni Y per prospettiva
    float p1BaseY = windowSize.y - p1ZoneHeight * 0.3f; // Più vicino al fondo
    float p2BaseY = windowSize.y * 0.15f; // Molto più in alto
    
    // Larghezza delle zone principali con prospettiva
    float p1MainZoneWidth = 3 * slotSizeP1.x + 2 * spacingP1;
    float p2MainZoneWidth = 3 * slotSizeP2.x + 2 * spacingP2;
    
    float p1StartX = centerX - p1MainZoneWidth / 2.0f;
    float p2StartX = centerX - p2MainZoneWidth / 2.0f;
    
    //------- GIOCATORE 1 (in basso, più grande e vicino) -------
    float p1MonsterY = p1BaseY - slotSizeP1.y;
    float p1SpellY = p1MonsterY + slotSizeP1.y + spacingP1;
    
    // 3 slot mostri giocatore 1
    for (int i = 0; i < 3; ++i) {
        float x = p1StartX + i * (slotSizeP1.x + spacingP1);
        slots.emplace_back(sf::Vector2f(x, p1MonsterY), monsterTexture, Slot::Type::Monster, slotSizeP1, p1Transf);
    }
    
    // 3 slot magie/trappole giocatore 1
    for (int i = 0; i < 3; ++i) {
        float x = p1StartX + i * (slotSizeP1.x + spacingP1);
        slots.emplace_back(sf::Vector2f(x, p1SpellY), spellTrapTexture, Slot::Type::SpellTrap, slotSizeP1, p1Transf);
    }

    //------- GIOCATORE 2 (in alto, più piccolo e lontano) -------
    float p2MonsterY = p2BaseY;
    float p2SpellY = p2MonsterY + slotSizeP2.y + spacingP2;
    
    // Crea la texture ruotata per le magie del giocatore 2
    createRotatedTexture(monsterTexture, rotatedMonsterTexture);
    
    for (int i = 0; i < 3; ++i) {
        float x = p2StartX + i * (slotSizeP2.x + spacingP2);
        slots.emplace_back(sf::Vector2f(x, p2MonsterY), rotatedMonsterTexture, Slot::Type::Monster, slotSizeP2, p2Transf);
    }
    
    createRotatedTexture(spellTrapTexture, rotatedSpellTrapTexture);
    
    for (int i = 0; i < 3; ++i) {
        float x = p2StartX + i * (slotSizeP2.x + spacingP2);
        slots.emplace_back(sf::Vector2f(x, p2SpellY), rotatedSpellTrapTexture, Slot::Type::SpellTrap, slotSizeP2, p2Transf);
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
    slots.emplace_back(sf::Vector2f(rightX, p1MonsterY), graveyardTexture, Slot::Type::Graveyard, slotSize, p1Transf);
    slots.emplace_back(sf::Vector2f(rightX, p1SpellY), deckTexture, Slot::Type::Deck, slotSize, p1Transf);

    // Crea la texture ruotata per le Extra e FieldSpell del giocatore 2
    createRotatedTexture(extraDeckTexture, rotatedExtraDeckTexture);
    createRotatedTexture(fieldSpellTexture, rotatedFieldSpellTexture);

    // Zone speciali giocatore 2 (destra del campo)
    slots.emplace_back(sf::Vector2f(rightX, p2MonsterY), rotatedExtraDeckTexture, Slot::Type::Extra, slotSize, p2Transf);
    slots.emplace_back(sf::Vector2f(rightX, p2SpellY), rotatedFieldSpellTexture, Slot::Type::FieldSpell, slotSize, p2Transf);

    // Extra Deck e Field Spell a sinistra
    slots.emplace_back(sf::Vector2f(leftX, p1MonsterY), fieldSpellTexture, Slot::Type::FieldSpell, slotSize, p1Transf);
    slots.emplace_back(sf::Vector2f(leftX, p1SpellY), extraDeckTexture, Slot::Type::Extra, slotSize, p1Transf);

     // Crea la texture ruotata per le Graveyard e Deck del giocatore 2
    createRotatedTexture(deckTexture, rotatedDeckTexture);
    createRotatedTexture(graveyardTexture, rotatedGraveyardTexture);

    slots.emplace_back(sf::Vector2f(leftX, p2MonsterY), rotatedDeckTexture, Slot::Type::Deck, slotSize, p2Transf);
    slots.emplace_back(sf::Vector2f(leftX, p2SpellY), rotatedGraveyardTexture, Slot::Type::Graveyard, slotSize, p2Transf);
}

sf::Vector2f Field::calculateSlotSize() const {
    // Calcola dimensioni slot in base alla finestra
    float availableWidth = windowSize.x * 0.72f; 
    float slotWidth = availableWidth / 5.0f; 
    
    // Mantieni proporzioni 128:180 dalla texture originale  
    float aspectRatio = 180.0f / 128.0f;
    float slotHeight = slotWidth * aspectRatio;
    
    // Aumentato ancora l'altezza massima
    float maxHeight = windowSize.y * 0.16f;
    if (slotHeight > maxHeight) {
        slotHeight = maxHeight;
        slotWidth = slotHeight / aspectRatio;
    }
    
    // Aumentato anche la dimensione minima
    float minHeight = 70.0f; 
    if (slotHeight < minHeight) {
        slotHeight = minHeight;
        slotWidth = slotHeight / aspectRatio;
    }
    
    return sf::Vector2f(slotWidth, slotHeight);
}

float Field::calculateSpacing() const { // Spaziatura proporzionale alla dimensione della finestra  
    return windowSize.x * 0.01f; 
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
    
    sf::Sprite sprite(original);
    
    // Imposta l'origine al centro dello sprite
    sprite.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
    
    // Posiziona lo sprite al centro del RenderTexture
    sprite.setPosition(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
    
    sprite.setRotation(sf::degrees(180.0f));
    
    // Renderizza lo sprite ruotato
    renderTexture.clear(sf::Color::Transparent);
    renderTexture.draw(sprite);
    renderTexture.display();
    
    // Copia il risultato nella texture di destinazione
    rotated = renderTexture.getTexture();
}


 