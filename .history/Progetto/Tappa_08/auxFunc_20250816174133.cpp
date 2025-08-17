#include "auxFunc.h"
#include "Field/Field.h"
#include <limits>

//-------------------------------- Funzione per il word wrapping del testo che preserva la formattazione --------------------------------//

std::string wrapText(const std::string& text, const sf::Font& font, unsigned int characterSize, float maxWidth) {
    std::string wrappedText;
    std::istringstream lines(text);
    std::string line;
    
    sf::Text tempText(font, "", characterSize);
    
    // Processa riga per riga per preservare la formattazione originale
    while (std::getline(lines, line)) {
        if (line.empty()) {
            // Preserva le righe vuote
            wrappedText += "\n";
            continue;
        }
        
        std::istringstream words(line);
        std::string word;
        std::string currentLine;
        
        while (words >> word) {
            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
            tempText.setString(testLine);
            
            if (tempText.getLocalBounds().size.x <= maxWidth) {
                currentLine = testLine;
            } else {
                if (!currentLine.empty()) {
                    wrappedText += currentLine + "\n";
                    currentLine = word;
                } else {
                    // Parola troppo lunga per una riga
                    wrappedText += word + "\n";
                }
            }
        }
        
        if (!currentLine.empty()) {
            wrappedText += currentLine + "\n";
        }
    }
    
    return wrappedText;
}

//-------------------------------- Funzioni di calcolo del layout --------------------------------//

sf::Vector2f calculateSlotSize(const sf::Vector2u& windowSize, 
                              float widthPercentage, 
                              float maxHeightPercentage, 
                              float minHeight) {
    // Calcola dimensioni slot in base alla finestra
    float availableWidth = windowSize.x * widthPercentage;
    float slotWidth = availableWidth / 5.0f; // 3 slot + 2 spaziature equivalenti
    
    // Mantieni proporzioni 128:180 dalla texture originale  
    float aspectRatio = 180.0f / 128.0f;
    float slotHeight = slotWidth * aspectRatio;
    
    // Limita l'altezza se troppo grande
    float maxHeight = windowSize.y * maxHeightPercentage;
    if (slotHeight > maxHeight) {
        slotHeight = maxHeight;
        slotWidth = slotHeight / aspectRatio;
    }
    
    // Assicurati che non sia troppo piccolo
    if (slotHeight < minHeight) {
        slotHeight = minHeight;
        slotWidth = slotHeight / aspectRatio;
    }
    
    return sf::Vector2f(slotWidth, slotHeight);
}

float calculateSpacing(const sf::Vector2u& windowSize, float spacingPercentage) {
    // Spaziatura proporzionale alla dimensione della finestra
    return windowSize.x * spacingPercentage;
}

float calculateZoneOffset(const sf::Vector2u& windowSize, 
                         const sf::Vector2f& slotSize, 
                         float spacing, 
                         float offsetPercentage) {
    // Calcola offset dinamico per evitare overflow e lasciare spazio per le carte
    float maxTotalHeight = 4 * slotSize.y + 3 * spacing; // 2 righe per giocatore + spaziature
    return std::min(windowSize.y * offsetPercentage, (windowSize.y - maxTotalHeight) / 5.0f);
}

float calculateMargin(const sf::Vector2u& windowSize, 
                     float marginPercentage, 
                     float minMargin) {
    // Margine dinamico dai bordi
    return std::max(windowSize.x * marginPercentage, minMargin);
}

//-------------------------------- Funzioni per l'interfaccia utente delle carte --------------------------------//

void showCardDetails(sf::RenderWindow& window, 
                    const Card& card, 
                    const sf::Font& font,
                    sf::Vector2f panelPos,
                    sf::Vector2f panelSize,
                    float scrollOffset) {
    // Ottieni la dimensione della finestra
    sf::Vector2u winSize = window.getSize();

    // Dimensioni e posizione scalate in base alla risoluzione (più piccolo)
    float panelWidth = std::max(panelSize.x, winSize.x * 0.18f); // almeno 18% larghezza
    float panelHeight = std::max(panelSize.y, winSize.y * 0.22f); // almeno 22% altezza
    sf::Vector2f scaledPanelSize(panelWidth, panelHeight);
    sf::Vector2f scaledPanelPos(
        std::max(panelPos.x, winSize.x * 0.15f),
        std::max(panelPos.y, winSize.y * 0.08f)
    );

    // Font size scalato (più piccolo)
    unsigned int detailFontSize = std::max(12u, static_cast<unsigned int>(winSize.y * 0.012f));
    unsigned int escFontSize = std::max(10u, static_cast<unsigned int>(winSize.y * 0.009f));


    // Valori della carta
    auto valuesOpt = card.getValues();
    int atk = 0, def = 0;
    if (valuesOpt.has_value()) {
        atk = valuesOpt.value().first;
        def = valuesOpt.value().second;
    }

    // Conversione enum in stringa
    auto typeToString = [](Type t) {
        switch(t) {
            case Type::Monster: return "Mostro";
            case Type::SpellTrap: return "Magia/Trappola";
            case Type::Deck: return "Deck";
            case Type::Graveyard: return "Cimitero";
            case Type::Extra: return "Extra";
            case Type::FieldSpell: return "Campo";
            default: return "Sconosciuto";
        }
    };
    auto attrToString = [](Attribute a) {
        switch(a) {
            case Attribute::None: return "Nessuno";
            case Attribute::Luce: return "Luce";
            case Attribute::Oscurita: return "Oscurita";
            case Attribute::Vento: return "Vento";
            case Attribute::Terra: return "Terra";
            case Attribute::Acqua: return "Acqua";
            case Attribute::Fuoco: return "Fuoco";
            case Attribute::Magia: return "Magia";
            case Attribute::Trappola: return "Trappola";
            default: return "Sconosciuto";
        }
    };
    auto featureToString = [](Feature f) {
        switch(f) {
            case Feature::Fusione: return "Fusione";
            case Feature::Rituale: return "Rituale";
            case Feature::Synchro: return "Synchro";
            case Feature::Xyz: return "Xyz";
            case Feature::Normale: return "Normale";
            case Feature::Effetto: return "Effetto";
            case Feature::Aqua: return "Aqua";
            case Feature::Bestia: return "Bestia";
            case Feature::BestiaAlata: return "Bestia Alata";
            case Feature::BestiaGuerriero: return "Bestia Guerriero";
            case Feature::Fata: return "Fata";
            case Feature::Drago: return "Drago";
            case Feature::Dinosauro: return "Dinosauro";
            case Feature::DivinitaBestia: return "Divinita Bestia";
            case Feature::Incantatore: return "Incantatore";
            case Feature::Insetto: return "Insetto";
            case Feature::Macchina: return "Macchina";
            case Feature::Pesce: return "Pesce";
            case Feature::Pianta: return "Pianta";
            case Feature::Pyro: return "Pyro";
            case Feature::Rettile: return "Rettile";
            case Feature::Roccia: return "Roccia";
            case Feature::Demone: return "Demone";
            case Feature::SerpenteMarino: return "Serpente Marino";
            case Feature::Tuono: return "Tuono";
            case Feature::Guerriero: return "Guerriero";
            case Feature::Zombie: return "Zombie";
            case Feature::Psichico: return "Psichico";
            case Feature::Tuner: return "Tuner";
            case Feature::Rapida: return "Rapida";
            case Feature::Continua: return "Continua";
            case Feature::Terreno: return "Terreno";
            case Feature::ControTrappola: return "Contro Trappola";
            default: return "Altro";
        }
    };

    // Pannello stile carta
    sf::RectangleShape detailPanel(scaledPanelSize);
    detailPanel.setFillColor(sf::Color(230, 200, 120, 230)); // dorato chiaro
    detailPanel.setOutlineColor(sf::Color(180, 140, 40));
    detailPanel.setOutlineThickness(4.f);
    detailPanel.setPosition(scaledPanelPos);
    window.draw(detailPanel);


    // --- Layout dinamico ---
    float y = scaledPanelPos.y + 8.f;
    float x = scaledPanelPos.x + 18.f;

    // Nome grande in alto
    sf::Text nameText(font, card.getName(), detailFontSize + 10);
    nameText.setFillColor(sf::Color::Black);
    nameText.setStyle(sf::Text::Bold);
    nameText.setPosition(sf::Vector2f(x, y));
    window.draw(nameText);
    y += nameText.getLocalBounds().size.y + 10.f;

    // Attributo in alto a destra
    std::string attrStr = attrToString(card.getAttribute());
    sf::Text attrText(font, attrStr, detailFontSize);
    attrText.setFillColor(sf::Color(80,80,80));
    attrText.setStyle(sf::Text::Bold);
    float attrCircleR = 22.f;
    sf::CircleShape attrCircle(attrCircleR);
    attrCircle.setFillColor(sf::Color(220,220,220));
    attrCircle.setOutlineColor(sf::Color(180,180,180));
    attrCircle.setOutlineThickness(2.f);
    attrCircle.setPosition(sf::Vector2f(scaledPanelPos.x + scaledPanelSize.x - attrCircleR*2 - 12.f, scaledPanelPos.y + 6.f));
    window.draw(attrCircle);
    attrText.setPosition(sf::Vector2f(attrCircle.getPosition().x + 7.f, attrCircle.getPosition().y + 7.f));
    window.draw(attrText);

    // Stelle livello/rango (max 12, font più piccolo) + numero accanto
    std::string stars = "";
    int nStars = std::min(12, card.getLevelOrRank().value_or(0));
    if(nStars > 0){
        for(int i=0; i<nStars; ++i) stars += " * ";
        stars += "  " + std::to_string(nStars);
        sf::Text starText(font, stars, detailFontSize-2);
        starText.setFillColor(sf::Color(255, 120, 0));
        starText.setPosition(sf::Vector2f(x, y));
        window.draw(starText);    
        y += starText.getLocalBounds().size.y + 10.f; 

    }
     
    // Tipo tra parentesi quadre sopra la descrizione
    std::string typeStr = "[" + std::string(typeToString(card.getType())) + "]";
    sf::Text typeText(font, typeStr, detailFontSize);
    typeText.setFillColor(sf::Color::Black);
    typeText.setStyle(sf::Text::Bold);
    typeText.setPosition(sf::Vector2f(x, y));
    window.draw(typeText);
    y += typeText.getLocalBounds().size.y + 8.f;

    // Features (sotto il tipo)
    const auto& feats = card.getFeatures();
    if (!feats.empty()) {
        std::string featStr = "(";
        for (size_t i = 0; i < feats.size(); ++i) {
            featStr += featureToString(feats[i]);
            if (i < feats.size() - 1) featStr += ", ";
        }
        featStr += ")";
        sf::Text featText(font, featStr, detailFontSize-1);
        featText.setFillColor(sf::Color(60,60,60));
        featText.setPosition(sf::Vector2f(x, y));
        window.draw(featText);
    y += featText.getLocalBounds().size.y + 8.f;
    }

    // Descrizione centrale con scroll e clipping (view solo sulla zona descrizione)
    float descX = x;
    float descY = y;
    float descW = scaledPanelSize.x - 36.f;
    float spazioATKDEFESC = 34.f;
    float descH = scaledPanelSize.y - (descY - scaledPanelPos.y) - spazioATKDEFESC;
    std::string wrappedDesc = wrapText(card.getDescription(), font, detailFontSize, descW);
    sf::Text descText(font, wrappedDesc, detailFontSize);
    descText.setFillColor(sf::Color::Black);
    float descTotalHeight = descText.getLocalBounds().size.y;
    // Limita lo scrollOffset per non far uscire il testo sopra o sotto
    float maxScroll = descTotalHeight - descH;
    if (maxScroll < 0) maxScroll = 0;
    if (scrollOffset < 0) scrollOffset = 0;
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
    // Clipping tramite view solo sulla descrizione (in pixel, non normalizzato)
    sf::View oldView = window.getView();
    sf::Vector2f descCenter(descX + descW/2.f, descY + descH/2.f);
    sf::Vector2f descSize(descW, descH);
    sf::View descView(descCenter, descSize);
    descView.setViewport(sf::FloatRect(
        sf::Vector2f((descX) / winSize.x, (descY) / winSize.y),
        sf::Vector2f((descW) / winSize.x, (descH) / winSize.y)
    ));
    window.setView(descView);
    descText.setPosition(sf::Vector2f(descX, descY - scrollOffset));
    window.draw(descText);
    window.setView(oldView);
    // Aggiorna y per posizionare ATK/DEF e ESC sempre sotto la descrizione
    y = descY + descH + 6.f;

    // ATK/DEF subito sotto la descrizione, a destra
    if (card.getType() == Type::Monster && card.getValues().has_value()) {
        auto [atk, def] = card.getValues().value();
        std::string stats = "ATK/" + std::to_string(atk) + "  DEF/" + std::to_string(def);
        sf::Text statsText(font, stats, detailFontSize + 1);
        statsText.setFillColor(sf::Color(40,40,40));
        statsText.setStyle(sf::Text::Bold);
        float statsW = statsText.getLocalBounds().size.x;
        statsText.setPosition(sf::Vector2f(scaledPanelPos.x + scaledPanelSize.x - statsW - 18.f, y));
        window.draw(statsText);
    }
    
    // Testo ESC subito sotto la descrizione, a sinistra
    sf::Text escTextObj(font, "Premi ESC per chiudere", escFontSize - 1);
    escTextObj.setFillColor(sf::Color(120, 80, 0));
    escTextObj.setPosition(sf::Vector2f(scaledPanelPos.x + 10.f, y));
    window.draw(escTextObj);
}
//-------------------------------- Funzioni per disegnare il testo della schermata di avvio --------------------------------//

void drawStartScreen(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2u& windowSize, sf::Texture& screenTexture, float elapsedTime) {
    // Sfondo della schermata di avvio
    sf::Sprite backgroundSprite(screenTexture);
    sf::Vector2u textureSize = screenTexture.getSize();
    float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(textureSize.x);
    float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(textureSize.y);
    backgroundSprite.setScale(sf::Vector2f(scaleX, scaleY));

    // Testo della schermata di avvio
    sf::Text titleText(font, "Premere il tasto INVIO per iniziare a giocare", 36);
    titleText.setStyle(sf::Text::Bold);

    // Animazione fade-in/fade-out
    float period = 2.5f; // secondi per ciclo completo
    float phase = std::fmod(elapsedTime, period) / period; // fmod è la stessa cosa di % ma funziona anche coi i float e double
    float alpha = 128 + 127 * std::sin(phase * 2 * 3.14159265f); // sfrutto una sinusoide per avere un periodo. In questo modo ogni tot periodo viene modificato l'alpha del valore per gestirne la trasparenza
    titleText.setFillColor(sf::Color(255, 255, 0, static_cast<unsigned char>(alpha)));

    // Centra il titolo orizzontalmente e in basso
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition(sf::Vector2f((windowSize.x - titleBounds.size.x) / 2.0f, windowSize.y - titleBounds.size.y - 125.0f));

    // Disegna tutto
    window.draw(backgroundSprite);
    window.draw(titleText);
}

//-------------------------------- Funzione per la gestione delle carte in mano --------------------------------//

void updateHandPositions(std::vector<Card>& cards, 
                        const sf::Vector2u& windowSize,
                        const sf::Vector2f& cardSize,
                        float spacing,
                        float y,
                        int maxHandSize) {
    if (cards.empty()) return;
    
    size_t numCards = cards.size();
    float currentCardWidth = cardSize.x;
    float currentSpacing = spacing;
    
    // Se abbiamo troppe carte, riduci la dimensione e la spaziatura
    if (numCards > static_cast<size_t>(maxHandSize)) {
        float scalingFactor = static_cast<float>(maxHandSize) / numCards;
        currentCardWidth = cardSize.x * scalingFactor;
        currentSpacing = spacing * scalingFactor;
        
        // Ricalcola anche la dimensione delle carte
        sf::Vector2f newCardSize(currentCardWidth, cardSize.y * scalingFactor);
        for (auto& card : cards) {
            card.setSize(newCardSize);
        }
    }
    
    float totalHandWidth = numCards * currentCardWidth + (numCards - 1) * currentSpacing;
    float startX = (windowSize.x - totalHandWidth) / 2.f;
    
    // Riposiziona tutte le carte
    for (size_t i = 0; i < cards.size(); ++i) {
        cards[i].setPosition(sf::Vector2f(startX + i * (currentCardWidth + currentSpacing), y));
    }
}

//-------------------------------- Funzione per calcolare la posizione finale post animazione --------------------------------//

sf::Vector2f setHandPos(std::vector<Card>& cards, Card& card, sf::Vector2u& windowSize, sf::Vector2f& cardSize, float spacing, float y, int HAND_MAXSIZE) {
    // Calcolo la posizione finale di card nella mano per far ciò inserisco momentaneamente la carta nella mano
    cards.push_back(card);
    updateHandPositions(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
    sf::Vector2f handPos = cards.back().getPosition();
    cards.pop_back(); // Rimuovo la carta dalla mano per non disegnarla due volte
    return handPos;
}

//-------------------------------- Funzioni per le zone di drop --------------------------------//

std::optional<sf::Vector2f> findSlotPosition(sf::Vector2f mousePos, const Card& card, Field& field, const sf::Vector2f& slotSize) {
    sf::Vector2f nearestPos = mousePos;
    
    // Aumentiamo l'area di rilevamento al 150% invece del 120%
    sf::Vector2f expandedSlotSize = slotSize * 1.5f;
    sf::Vector2f halfSize = expandedSlotSize / 2.f;
    
    Type cardType = card.getType();
    
    switch (cardType){
        case Type::Monster:
            // Cerca negli slot mostri di P1
            for (int i = 0; i < 5; ++i) {
                sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, 1, i);
                if (slotPos != sf::Vector2f(0.f, 0.f)) { // Verifica che lo slot esista
                    Slot* slot = field.getSlotByPosition(slotPos);
                    if (slot && !slot->isOccupied()) {
                        // Calcola se il mouse è nell'area dello slot
                        if (mousePos.x >= slotPos.x - halfSize.x && mousePos.x <= slotPos.x + halfSize.x &&
                            mousePos.y >= slotPos.y - halfSize.y && mousePos.y <= slotPos.y + halfSize.y) {
                            return slotPos;
                        }
                    }
                }
            }
            break;

        case Type::SpellTrap:
            {
                // Cerca negli slot magie/trappole di P1
                for (int i = 0; i < 5; ++i) {
                    sf::Vector2f slotPos = field.getSlotPosition(Type::SpellTrap, 1, i);
                    if (slotPos != sf::Vector2f(0.f, 0.f)) {
                        Slot* slot = field.getSlotByPosition(slotPos);
                        if (slot && !slot->isOccupied()) {
                            if (mousePos.x >= slotPos.x - halfSize.x && mousePos.x <= slotPos.x + halfSize.x &&
                                mousePos.y >= slotPos.y - halfSize.y && mousePos.y <= slotPos.y + halfSize.y) {
                                return slotPos;
                            }
                        }
                    }
                }
                
                // Controlla anche lo slot FieldSpell per le carte SpellTrap
                sf::Vector2f fieldPos = field.getSlotPosition(Type::FieldSpell, 1);
                if (fieldPos != sf::Vector2f(0.f, 0.f)) {
                    Slot* slot = field.getSlotByPosition(fieldPos);
                    if (slot && !slot->isOccupied()) {
                        if (mousePos.x >= fieldPos.x - halfSize.x && mousePos.x <= fieldPos.x + halfSize.x &&
                            mousePos.y >= fieldPos.y - halfSize.y && mousePos.y <= fieldPos.y + halfSize.y) {
                            return fieldPos;
                        }
                    }
                }
            }
            break;

        case Type::FieldSpell:
            {
                sf::Vector2f fieldPos = field.getSlotPosition(Type::FieldSpell, 1);
                if (fieldPos != sf::Vector2f(0.f, 0.f)) {
                    Slot* slot = field.getSlotByPosition(fieldPos);
                    if (slot && !slot->isOccupied()) {
                        if (mousePos.x >= fieldPos.x - halfSize.x && mousePos.x <= fieldPos.x + halfSize.x &&
                            mousePos.y >= fieldPos.y - halfSize.y && mousePos.y <= fieldPos.y + halfSize.y) {
                            return fieldPos;
                        }
                    }
                }
            }
            break;

        default:
            break;
    }
    
    return std::nullopt; // Nessuna posizione valida trovata
}

//------------------------------- Funzione per la schermata Home --------------------------------//

void drawHomeScreen(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2u& windowSize, sf::Texture& homeTexture, const std::string& deckName) {
    // Sfondo
    sf::Sprite backgroundSprite(homeTexture);
    sf::Vector2u textureSize = homeTexture.getSize();
    float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(textureSize.x);
    float scaleY = (static_cast<float>(windowSize.y) / static_cast<float>(textureSize.y)) * 1.20f;;
    backgroundSprite.setScale(sf::Vector2f(scaleX, scaleY));
    window.draw(backgroundSprite);

    // Scritta nome deck selezionato in alto
    unsigned int deckFontSize = std::max(22u, static_cast<unsigned int>(windowSize.y * 0.03f));
    sf::Text deckText(font, "Deck: " + deckName, deckFontSize);
    deckText.setFillColor(sf::Color(255, 255, 180));
    deckText.setStyle(sf::Text::Bold);
    deckText.setOutlineColor(sf::Color(120, 80, 0, 180));
    deckText.setOutlineThickness(2.f);
    
    // Centra orizzontalmente, posiziona in alto
    sf::FloatRect deckBounds = deckText.getLocalBounds();
    deckText.setPosition(sf::Vector2f((windowSize.x - deckBounds.size.x) / 2.0f, windowSize.y * 0.04f));
    window.draw(deckText);

    // --- PARTICELLE ANIMATE ---
    static std::vector<sf::CircleShape> particles;
    static std::vector<float> particleAlpha;
    static std::vector<sf::Vector2f> particleVel;
    static sf::Clock particleClock;
    static sf::Clock spawnClock;
    float dt = particleClock.restart().asSeconds();
    // Aggiorna particelle
    for (size_t i = 0; i < particles.size(); ++i) {
        sf::Vector2f pos = particles[i].getPosition();
        pos += particleVel[i] * dt;
        particles[i].setPosition(pos);
        float alpha = particleAlpha[i];
        alpha -= dt * 60.f; // fade out
        if (alpha < 0) alpha = 0;
        particleAlpha[i] = alpha;
        sf::Color c = particles[i].getFillColor();
    c.a = static_cast<unsigned char>(alpha);
        particles[i].setFillColor(c);
    }
    // Rimuovi particelle morte
    for (int i = (int)particles.size()-1; i >= 0; --i) {
        if (particleAlpha[i] <= 0) {
            particles.erase(particles.begin()+i);
            particleAlpha.erase(particleAlpha.begin()+i);
            particleVel.erase(particleVel.begin()+i);
        }
    }
    // Spawna nuove particelle
    if (spawnClock.getElapsedTime().asSeconds() > 0.08f) {
        spawnClock.restart();
        int nNew = rand()%3+1;
        for (int i=0; i<nNew; ++i) {
            float x = static_cast<float>(rand()%windowSize.x);
            float y = static_cast<float>(rand()%windowSize.y);
            float r = 1.5f + static_cast<float>(rand()%20)/10.f;
            sf::CircleShape p(r);
            p.setPosition(sf::Vector2f(x, y));
            p.setFillColor(sf::Color(255, 140, 40, 255));
            particles.push_back(p);
            particleAlpha.push_back(255.f);
            float vx = -20.f + static_cast<float>(rand()%41);
            float vy = -20.f + static_cast<float>(rand()%41);
            particleVel.push_back(sf::Vector2f(vx, vy));
        }
    }
    // Disegna particelle
    for (auto& p : particles) window.draw(p);

    // Voci menu
    std::vector<std::string> menuItems = {"Gioca", "Seleziona Deck"};
    unsigned int fontSize = std::max(36u, static_cast<unsigned int>(windowSize.y * 0.045f));
    float leftMargin = windowSize.x * 0.08f;
    float topMargin = windowSize.y * 0.22f;
    float spacing = windowSize.y * 0.08f;

    for (size_t i = 0; i < menuItems.size(); ++i) {
        sf::Text menuText(font, menuItems[i] , fontSize);
        menuText.setFillColor(sf::Color::White);
        menuText.setStyle(sf::Text::Bold);
        menuText.setOutlineColor(sf::Color(0,0,0,180));
        menuText.setOutlineThickness(3.f);
        menuText.setPosition(sf::Vector2f(leftMargin, topMargin + i * spacing));
        window.draw(menuText);
    }

}