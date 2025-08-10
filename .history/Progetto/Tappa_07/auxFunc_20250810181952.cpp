#include "auxFunc.h"
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
    float panelWidth = std::max(panelSize.x, winSize.x * 0.13f); // almeno 13% larghezza
    float panelHeight = std::max(panelSize.y, winSize.y * 0.15f); // almeno 15% altezza
    sf::Vector2f scaledPanelSize(panelWidth, panelHeight);
    sf::Vector2f scaledPanelPos(
        std::max(panelPos.x, winSize.x * 0.15f),
        std::max(panelPos.y, winSize.y * 0.08f)
    );

    // Font size scalato (più piccolo)
    unsigned int detailFontSize = std::max(12u, static_cast<unsigned int>(winSize.y * 0.012f));
    unsigned int escFontSize = std::max(10u, static_cast<unsigned int>(winSize.y * 0.009f));

    // Valori della carta
    auto [atk, def] = card.getValues();

    // Pannello dei dettagli
    sf::RectangleShape detailPanel(scaledPanelSize);
    detailPanel.setFillColor(sf::Color(0, 0, 0, 200));
    detailPanel.setOutlineColor(sf::Color::White);
    detailPanel.setOutlineThickness(2.f);
    detailPanel.setPosition(scaledPanelPos);

    // Creo la stringa con il testo principale 
    std::string detailRaw = "DETTAGLI CARTA\n\n";
    detailRaw += "Nome: " + card.getName() + "\n\n";
    detailRaw += "Descrizione:\n" + card.getDescription() + "\n\n";
    detailRaw += "ATK: " + std::to_string(atk) + "\n";
    detailRaw += "DEF: " + std::to_string(def);

    // Applica word wrapping al testo principale
    float textMaxWidth = scaledPanelSize.x - 40.f;
    std::string wrappedText = wrapText(detailRaw, font, detailFontSize, textMaxWidth);

    // Testo dettagli
    sf::Text detailTextObj(font, wrappedText, detailFontSize);
    detailTextObj.setFillColor(sf::Color::White);
    detailTextObj.setPosition(sf::Vector2f(scaledPanelPos.x + 10.f, scaledPanelPos.y + 10.f - scrollOffset));

    // Testo ESC
    sf::Text escTextObj(font, "Premi ESC per chiudere", escFontSize);
    escTextObj.setFillColor(sf::Color::Yellow);
    escTextObj.setPosition(sf::Vector2f(scaledPanelPos.x + 10.f, scaledPanelPos.y + scaledPanelSize.y - 25.f));

    // Disegna prima il pannello 
    window.draw(detailPanel);

    // Salva la view corrente
    sf::View originalView = window.getView();
    // View limitata al pannello per il clipping del testo scrollabile
    sf::View textView;
    textView.setSize(sf::Vector2f(scaledPanelSize.x - 20.f, scaledPanelSize.y - 50.f));
    textView.setCenter(sf::Vector2f(scaledPanelPos.x + scaledPanelSize.x/2.f, scaledPanelPos.y + (scaledPanelSize.y - 50.f)/2.f + 10.f));
    textView.setViewport(sf::FloatRect(
        sf::Vector2f((scaledPanelPos.x + 10.f) / winSize.x, (scaledPanelPos.y + 10.f) / winSize.y),
        sf::Vector2f((scaledPanelSize.x - 20.f) / winSize.x, (scaledPanelSize.y - 50.f) / winSize.y)
    ));

    window.setView(textView);
    window.draw(detailTextObj);
    window.setView(originalView);
    window.draw(escTextObj);
}

//-------------------------------- Funzioni per disegnare il testo della schermata di avvio --------------------------------//

void drawStartScreen(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2u& windowSize) {
    // Titolo del gioco
    sf::Text titleText(font, "IU-GHI-WOO", 36);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setStyle(sf::Text::Bold);
    
    // Centra il titolo orizzontalmente
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition(sf::Vector2f((windowSize.x - titleBounds.size.x) / 2.0f, 50.0f));
    
    // Istruzioni del gioco
    std::string instructions = 
        "Come giocare:\n\n"
        "- Dopo la pressione del tasto INVIO, attendi qualche secondo dopo i quali le carte verranno pescate\n"
        "- Nel campo, clicca sulle carte per visualizzare i dettagli della carta\n"
        "- Usa la rotella del mouse per scorrere i dettagli \n"
        "- Premi ESC per chiudere i dettagli delle carte, oppure premi in qualsiasi altro punto\n"
        
        "Premi INVIO per iniziare!";
    
    sf::Text instructionsText(font, instructions, 25);
    instructionsText.setFillColor(sf::Color::White);
    
    // Posiziona le istruzioni
    sf::FloatRect instrBounds = instructionsText.getLocalBounds();
    instructionsText.setPosition(sf::Vector2f((windowSize.x - instrBounds.size.x) / 2.0f, 150.0f));
    
    // Disegna tutto
    window.draw(titleText);
    window.draw(instructionsText);
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

bool isValidDropPosition(sf::Vector2f mousePos, const Card& card, const Field& field, const sf::Vector2f& slotSize) {
    // Calcola l'area di ogni slot (espansa un po' per facilità di drop)
    sf::Vector2f expandedSlotSize = slotSize * 1.2f; // 20% più grande per tolleranza
    sf::Vector2f halfSize = expandedSlotSize / 2.f;
    
    // Controlla se è un mostro (ha attacco o difesa)
    bool isMonster = (card.getAttack() > 0 || card.getDefense() > 0);
    
    if (isMonster) {
        // Controlla gli slot mostri di P1
        for (int i = 0; i < 5; ++i) {
            sf::Vector2f slotPos = field.getSlotPosition(Slot::Type::Monster, 1, i);
            if (mousePos.x >= slotPos.x - halfSize.x && mousePos.x <= slotPos.x + halfSize.x &&
                mousePos.y >= slotPos.y - halfSize.y && mousePos.y <= slotPos.y + halfSize.y) {
                return true;
            }
        }
    } else {
        // È una spell/trap - controlla gli slot spell/trap di P1
        for (int i = 0; i < 5; ++i) {
            sf::Vector2f slotPos = field.getSlotPosition(Slot::Type::SpellTrap, 1, i);
            if (mousePos.x >= slotPos.x - halfSize.x && mousePos.x <= slotPos.x + halfSize.x &&
                mousePos.y >= slotPos.y - halfSize.y && mousePos.y <= slotPos.y + halfSize.y) {
                return true;
            }
        }
        
        // Controlla anche il field spell slot
        sf::Vector2f fieldPos = field.getSlotPosition(Slot::Type::FieldSpell, 1);
        if (mousePos.x >= fieldPos.x - halfSize.x && mousePos.x <= fieldPos.x + halfSize.x &&
            mousePos.y >= fieldPos.y - halfSize.y && mousePos.y <= fieldPos.y + halfSize.y) {
            return true;
        }
    }
    
    return false; // Nessuno slot valido trovato
}

sf::Vector2f findSlotPosition(sf::Vector2f mousePos, const Card& card, const Field& field, const sf::Vector2f& slotSize) {
    sf::Vector2f nearestPos = mousePos;
    float minDistance = std::numeric_limits<float>::max();
    
    // Calcola l'area di ogni slot
    sf::Vector2f expandedSlotSize = slotSize * 1.2f;
    sf::Vector2f halfSize = expandedSlotSize / 2.f;
    
    Type cardType = card.getType();
    
    switch (cardType)
    {
    case constant expression:
        /* code */
        break;
    
    default:
        break;
    }

    if (isMonster) {
        // Cerca negli slot mostri di P1
        for (int i = 0; i < 5; ++i) {
            sf::Vector2f slotPos = field.getSlotPosition(Slot::Type::Monster, 1, i);
            
            // Controlla se il mouse è dentro questo slot
            if (mousePos.x >= slotPos.x - halfSize.x && mousePos.x <= slotPos.x + halfSize.x &&
                mousePos.y >= slotPos.y - halfSize.y && mousePos.y <= slotPos.y + halfSize.y) {
                return slotPos; // Ritorna il centro dello slot
            }
        }
    } else {
        // Cerca negli slot spell/trap di P1
        for (int i = 0; i < 5; ++i) {
            sf::Vector2f slotPos = field.getSlotPosition(Slot::Type::SpellTrap, 1, i);
            
            if (mousePos.x >= slotPos.x - halfSize.x && mousePos.x <= slotPos.x + halfSize.x &&
                mousePos.y >= slotPos.y - halfSize.y && mousePos.y <= slotPos.y + halfSize.y) {
                return slotPos;
            }
        }
        
        // Controlla il field spell slot
        sf::Vector2f fieldPos = field.getSlotPosition(Slot::Type::FieldSpell, 1);
        if (mousePos.x >= fieldPos.x - halfSize.x && mousePos.x <= fieldPos.x + halfSize.x &&
            mousePos.y >= fieldPos.y - halfSize.y && mousePos.y <= fieldPos.y + halfSize.y) {
            return fieldPos;
        }
    }
    
    return mousePos; // Se non trova uno slot valido, ritorna la posizione del mouse
}