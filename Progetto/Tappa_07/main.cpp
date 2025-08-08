#include <SFML/Graphics.hpp>
#include "Field/Field.h"
#include "Deck/Deck.h"
#include "DrawAnimation/DrawAnimation.h"
#include "../resources/GameState.h"
#include "auxFunc.h"
#include <vector>
#include <iostream>
#define P1 1 
#define P2 2
#define DECK_SIZE 30 // Numero di carte nel deck
#define HAND_MAXSIZE 7 
#define CARD_MAXOFFSET 50.f
#define MOVEMENT_SPEED 2000.f


int main(){
    std::vector<DrawAnimation> animations;
    std::vector<Card> cardsToDraw; // Carte da pescare (coda)
    GameState gamestate = GameState::Intro;
    bool mousePressed = false; 

    // Definisci le dimensioni della finestra
    sf::Vector2u windowSize(2500, 1400);
    sf::RenderWindow window(sf::VideoMode(windowSize), "Progetto Tappa 07");

    // Carico le texture necessarie per il campo di gioco
    sf::Texture fieldTexture("../../Progetto/resources/backgroundTexture.jpg");
    sf::Texture monsterTexture("../../Progetto/resources/monsterText.png");
    sf::Texture spellTrapTexture("../../Progetto/resources/spellTrapTexture.png");
    sf::Texture deckTexture("../../Progetto/resources/deckTexture.png");
    sf::Texture graveyardTexture("../../Progetto/resources/graveTexture.png");
    sf::Texture extraDeckTexture("../../Progetto/resources/ExtraDeckTexture.png");
    sf::Texture fieldSpellTexture("../../Progetto/resources/fieldSpell.png");

    // Carico le texture necessarie per le carte
    sf::Texture textureFlipped(("../../Progetto/resources/Texture1.png"));
    sf::Texture textureNonFlipped(("../../Progetto/resources/CardNotSet.jpg"));

    // Carico il font per le etichette delle carte
    sf::Font detailFont;
    if (!detailFont.openFromFile("../../Progetto/resources/ITCKabelStdDemi.TTF")) {

        if (!detailFont.openFromFile("C:/Windows/Fonts/calibri.ttf")) {
            std::cerr << "ERRORE: Impossibile caricare nessun font!" << std::endl;
        }
    }

    // Crea il campo di gioco con dimensioni dinamiche
    Field field(
        fieldTexture, 
        monsterTexture, 
        spellTrapTexture, 
        deckTexture, 
        graveyardTexture, 
        extraDeckTexture, 
        fieldSpellTexture, 
        windowSize
    );        
    
    // Calcola le dimensioni per le carte del deck (leggermente più piccole dello slot)
    sf::Vector2f slotSize = calculateSlotSize(windowSize);
    float deckScaleFactor = 0.9f; 
    sf::Vector2f deckCardSize(slotSize.x * deckScaleFactor, slotSize.y * deckScaleFactor);

    // Posizione dello slot del deck
    sf::Vector2f deckSlotPos = field.getSlotPosition(Slot::Type::Deck, P1); 

    // Creo il deck
    Deck deck(deckSlotPos, deckCardSize, slotSize, textureFlipped, DECK_SIZE);

    // Creo le carte in mano, inizialmente il vettore raffigurante le carte in mano è vuoto
    std::vector<Card> cards;
    cards.reserve(10); 
    const int initialCard = 5;
    // Prepara la coda delle carte da pescare
    for(int i=0; i<initialCard; ++i){
        if(!deck.isEmpty()){
            Card drawnCard = deck.drawCard();
            cardsToDraw.push_back(drawnCard);
        }
    }

    //Calcolo la dimensione della carta rendendola un po' più grande delle dimensioni degli slot e prendo i parametri di posizione e spaziatura
    float scaleFactor = 1.1f;
    sf::Vector2f cardSize(slotSize.x * scaleFactor, slotSize.y * scaleFactor);
    float spacing = 15.f;
    float totalHandWidth = initialCard * cardSize.x + (initialCard - 1) * spacing;
    float startX = (windowSize.x - totalHandWidth) / 2.f;
    
    //Ripristino la posizione originale delle carte
    float y = windowSize.y - cardSize.y - 15.f; 

    std::optional<size_t> selectedCardIndex; // Indice della carta selezionata
    float scrollOffset = 0.f; //Offset per lo scroll del testo dei dettagli della carta

    float fieldAlpha = 0.f; // Inizialmente il campo di gioco è trasparente
    float fieldOffset = 200.f; //Inizialmente ci troviamo verso l'alto dello schermo
    float deckAlpha = 0.f;

    while(window.isOpen()){

        //1. Gestione degli eventi
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                std::cout << "Richiesta di chiusura..." << std::endl;
                window.close();
            }

            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                if(keyPressed->code == sf::Keyboard::Key::Enter && gamestate == GameState::Intro) {
                    std::cout << "Passaggio allo stato FieldVisible..." << std::endl;
                    gamestate = GameState::FieldVisible; 
                }
            }


            if (const auto *mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Mouse::Button but = mouseButton->button;
                if(but == sf::Mouse::Button::Left) {
                    mousePressed = true;
                }
            }
            if (const auto *mouseButton = event->getIf<sf::Event::MouseButtonReleased>()) {
                sf::Mouse::Button but = mouseButton->button;
                if(but == sf::Mouse::Button::Left) {
                    mousePressed = false;
                }
            }

            // Se il mouse è premuto, aggiorna la selezione della carta sotto il mouse
            if (mousePressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                bool cardClicked = false;
                for (size_t i = 0; i < cards.size(); ++i) {
                    if (cards[i].isClicked(mousePos)) {
                        if (!selectedCardIndex.has_value() || selectedCardIndex.value() != i) {
                            scrollOffset = 0.f;
                        }
                        selectedCardIndex = i;
                        cardClicked = true;
                        break;
                    }
                }
                if (!cardClicked) {
                    selectedCardIndex.reset();
                    scrollOffset = 0.f;
                }
            }

            // Gestione tasto ESC per deselezionare
            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    selectedCardIndex.reset();
                    scrollOffset = 0.f; // Resetta lo scroll anche con ESC
                    std::cout << "Carta deselezionata con ESC" << std::endl;
                }
            }
             
            // Scroll mouse
            if (const auto* mouseScroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (selectedCardIndex.has_value()) {
                    scrollOffset -= mouseScroll->delta * 20.f;
                    scrollOffset = std::max(0.f, scrollOffset); // niente scroll negativo
                }
            }
        }

        //2. Aggiornamento della logica del gioco
        static sf::Clock clock;
        float deltaTime = clock.restart().asSeconds();

        if(gamestate == GameState::FieldVisible) {
            float scrollSpeed = 300.f;
            float fadeSpeed = 200.f;
                
            fieldOffset -= scrollSpeed * deltaTime; 
            fieldAlpha += fadeSpeed * deltaTime;
            
            //Se superiamo i limiti, blocco gli offset
            if(fieldOffset <= 0.f) fieldOffset = 0.f; 
            if(fieldAlpha >= 255.f) fieldAlpha = 255.f;

            if(fieldOffset == 0.f && fieldAlpha == 255.f) {
                field.setAnimationFinished();

                deckAlpha += fadeSpeed * deltaTime;
                if(deckAlpha >= 255.f) deckAlpha = 255.f;

                if(deckAlpha >= 255.f) {
                    deck.setAnimationFinished();
                    std::cout << "Passaggio allo stato Playing..." << std::endl;
                    gamestate = GameState::Playing; // Passa allo stato di gioco dopo 1 secondo

                    // Avvia la prima animazione di pescata (una alla volta)
                    if (!cardsToDraw.empty() && animations.empty()) {
                        Card nextCard = cardsToDraw.front();
                        cardsToDraw.erase(cardsToDraw.begin());
                        DrawAnimation anim(
                            nextCard, DrawAnimationPhases::MovingOut, deckSlotPos, 
                               sf::Vector2f(windowSize.x / 2.f - cardSize.x / 2.f, windowSize.y / 2.f - cardSize.y / 2.f)
                        );
                        animations.push_back(anim);
                    }
                }
            }
        }

        if(gamestate == GameState::Playing){
            // Aggiorna solo la prima animazione (una alla volta)
            if (!animations.empty()) {
                animations.front().update(MOVEMENT_SPEED, deltaTime, textureNonFlipped, cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                if (animations.front().isFinished()) {
                    cards.push_back(animations.front().getCard());
                    animations.erase(animations.begin());
                    updateHandPositions(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                    // Avvia la prossima animazione se ci sono altre carte da pescare
                    if (!cardsToDraw.empty()) {
                        Card nextCard = cardsToDraw.front();
                        cardsToDraw.erase(cardsToDraw.begin());
                        DrawAnimation anim(
                            nextCard, DrawAnimationPhases::MovingOut, deckSlotPos,
                               sf::Vector2f(windowSize.x / 2.f - cardSize.x / 2.f, windowSize.y / 2.f - cardSize.y / 2.f)
                        );
                        animations.push_back(anim);
                    }
                }
            }
        }

        //Gestione del sollevamento delle carte in mano
        float targetCardOffset = 0.f;
        float offsetSpeed = 200.f; 
        for(size_t i = 0; i<cards.size(); ++i){
            float currentOffset = cards[i].getOffset();
            if(selectedCardIndex.has_value() && selectedCardIndex.value() == i) targetCardOffset = CARD_MAXOFFSET; // l'offset di destinazione per la carta selezionata è al massimo
            else targetCardOffset = 0.f; 

            //Dopo aver calcolato l'offset di destinazione mi sposto piano piano verso di esso
            if(currentOffset < targetCardOffset) {
                currentOffset = std::min(currentOffset + offsetSpeed * deltaTime, targetCardOffset); 
                cards[i].setOffset(currentOffset);
            } else { 
                // Se l'offset corrente è maggiore del target, lo riduco (torno indietro)
                currentOffset = std::max(currentOffset - offsetSpeed * deltaTime, targetCardOffset);
                cards[i].setOffset(currentOffset);
            }    
        }

        //3. Blocco Rendering
        window.clear(sf::Color::Black);

        // Ottieni la posizione del mouse
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        
        // Disegna sempre il campo di gioco (che include lo sfondo)
        field.draw(window, mousePos, gamestate, fieldAlpha, fieldOffset);
        
        // Se siamo nello stato Intro, disegna il testo sopra lo sfondo
        if(gamestate == GameState::Intro) drawStartScreen(window, detailFont, windowSize);
        
        else {
            
            // Disegna il deck 
            if(field.isAnimationFinished()) deck.draw(window, mousePos, detailFont, deckSlotPos, slotSize, gamestate, deckAlpha);

            //Disegna le animazioni delle carte
            if(deck.isAnimationFinished()) for(auto& anim : animations) anim.draw(window);
            
            for (auto& card : cards){
                sf::Vector2f originalPos = card.getPosition();
                float offset = card.getOffset();
                card.setPosition(originalPos - sf::Vector2f(0.f, offset));
                card.draw(window);
                card.setPosition(originalPos); 
            } 

            // Se c'è una carta selezionata, mostra i dettagli - Non influisce sullo stato del gioco, ma solo sulla visualizzazione (rendering)
            if (selectedCardIndex.has_value()) {
                const Card& selectedCard = cards[selectedCardIndex.value()];
                sf::Vector2f panelPos{400.f, 150.f};
                sf::Vector2f panelSize{300.f, 200.f};
                showCardDetails(window, selectedCard, detailFont, panelPos, panelSize, scrollOffset);
            }
        }
        
        window.display();
    }
    return 0;
}