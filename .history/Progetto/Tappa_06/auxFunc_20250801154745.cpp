#include "auxFunc.h"

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
    
    // Valori della carta
    auto [atk, def] = card.getValues();

    // Pannello dei dettagli
    sf::RectangleShape detailPanel(panelSize);
    detailPanel.setFillColor(sf::Color(0, 0, 0, 200));
    detailPanel.setOutlineColor(sf::Color::White);
    detailPanel.setOutlineThickness(2.f);
    detailPanel.setPosition(panelPos);

    // Creo la stringa con il testo principale 
    std::string detailRaw = "DETTAGLI CARTA\n\n";
    detailRaw += "Nome: " + card.getName() + "\n\n";
    detailRaw += "Descrizione:\n" + card.getDescription() + "\n\n";
    detailRaw += "ATK: " + std::to_string(atk) + "\n";
    detailRaw += "DEF: " + std::to_string(def);

    // Applica word wrapping al testo principale
    float textMaxWidth = panelSize.x - 40.f; // Margine di 20px per lato
    std::string wrappedText = wrapText(detailRaw, font, 14, textMaxWidth);

    // Partendo dalla stringa originale, dopo averne fatto il wrapping, creo l'effettivo oggetto Text (il testo scrollabile effettivo)
    sf::Text detailTextObj(font, wrappedText, 14);
    detailTextObj.setFillColor(sf::Color::White);
    detailTextObj.setPosition(sf::Vector2f(panelPos.x + 10.f, panelPos.y + 10.f - scrollOffset));

    // Crea il testo ESC fisso in fondo al pannello
    sf::Text escTextObj(font, "Premi ESC per chiudere", 12);
    escTextObj.setFillColor(sf::Color::Yellow);
    escTextObj.setPosition(sf::Vector2f(panelPos.x + 10.f, panelPos.y + panelSize.y - 25.f));

    // Disegna prima il pannello 
    window.draw(detailPanel);

    // Salva la view corrente
    sf::View originalView = window.getView();
    
    // Crea una view limitata al pannello per il clipping del testo scrollabile
    // Riduciamo l'altezza per lasciare spazio al testo ESC
    sf::View textView;
    textView.setSize(sf::Vector2f(panelSize.x - 20.f, panelSize.y - 50.f)); // Più spazio in basso per ESC
    textView.setCenter(sf::Vector2f(panelPos.x + panelSize.x/2.f, panelPos.y + (panelSize.y - 50.f)/2.f + 10.f));
    textView.setViewport(sf::FloatRect(
        sf::Vector2f((panelPos.x + 10.f) / window.getSize().x, (panelPos.y + 10.f) / window.getSize().y),
        sf::Vector2f((panelSize.x - 20.f) / window.getSize().x, (panelSize.y - 50.f) / window.getSize().y)
    ));

    // Imposta la view per il testo scrollabile e disegnalo
    window.setView(textView);
    window.draw(detailTextObj);
    
    // Ripristina la view originale per disegnare il testo ESC fisso
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

//-------------------------------- Funzioni per le animazioni delle carte --------------------------------//

void startCardDrawAnimation(CardDrawAnimation& animation,
                           const Card& card,
                           const sf::Vector2f& startPos,
                           const sf::Vector2f& targetPos,
                           sf::Texture* flippedTexture,
                           sf::Texture* normalTexture,
                           float duration) {
    animation.card = card;
    animation.startPos = startPos;
    animation.targetPos = targetPos;
    animation.currentPos = startPos;
    animation.progress = 0.0f;
    animation.duration = duration;
    animation.flippedTexture = flippedTexture;
    animation.normalTexture = normalTexture;
    animation.isFlipping = false;
    animation.isActive = true;
    
    // Inizia con la texture coperta
    animation.card.setTexture(*flippedTexture);
}

bool updateCardDrawAnimation(CardDrawAnimation& animation, 
                            float deltaTime, 
                            const sf::Vector2u& windowSize) {
    if (!animation.isActive) return true;
    
    animation.progress += deltaTime / animation.duration;
    
    if (animation.progress >= 1.0f) {
        animation.progress = 1.0f;
        animation.currentPos = animation.targetPos;
        animation.isActive = false;
        return true; // Animazione completata
    }
    
    // Fase 1: Muovi la carta all'indietro fino a uscire dalla finestra (0.0 - 0.3)
    if (animation.progress <= 0.3f) {
        float phase1Progress = animation.progress / 0.3f;
        // Interpola verso una posizione dietro la finestra
        sf::Vector2f exitPos(animation.startPos.x, -200.0f); // Esce dalla parte superiore
        animation.currentPos = sf::Vector2f(
            animation.startPos.x + (exitPos.x - animation.startPos.x) * phase1Progress,
            animation.startPos.y + (exitPos.y - animation.startPos.y) * phase1Progress
        );
    }
    // Fase 2: Flip della texture e riposizionamento istantaneo (0.3 - 0.4)
    else if (animation.progress <= 0.4f && !animation.isFlipping) {
        animation.isFlipping = true;
        animation.card.setTexture(*animation.normalTexture);
        
        // Imposta il rettangolo di texture per mostrare l'intera texture
        sf::Vector2u texSize = animation.normalTexture->getSize();
        animation.card.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texSize.x, texSize.y)));
        
        // Riposiziona la carta dall'altra parte della finestra
        animation.currentPos = sf::Vector2f(animation.targetPos.x, -200.0f);
    }
    // Fase 3: Muovi la carta verso la posizione finale (0.4 - 1.0)
    else if (animation.progress > 0.4f) {
        float phase3Progress = (animation.progress - 0.4f) / 0.6f;
        // Usa easing per un movimento più fluido
        float easedProgress = phase3Progress * phase3Progress * (3.0f - 2.0f * phase3Progress); // smoothstep
        
        sf::Vector2f startPhase3(animation.targetPos.x, -200.0f);
        animation.currentPos = sf::Vector2f(
            startPhase3.x + (animation.targetPos.x - startPhase3.x) * easedProgress,
            startPhase3.y + (animation.targetPos.y - startPhase3.y) * easedProgress
        );
    }
    
    // Aggiorna la posizione della carta
    animation.card.setPosition(animation.currentPos);
    
    return false; // Animazione in corso
}

void drawCardAnimation(sf::RenderWindow& window, const CardDrawAnimation& animation) {
    if (animation.isActive) {
        animation.card.draw(window);
    }
}
