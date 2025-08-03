#include <SFML/Graphics.hpp>
#include "Field/Field.h"
#include "Deck/Deck.h"
#include "../resources/GameState.h"
#include "auxFunc.h"
#include <vector>
#include <iostream>
#define P1 1 
#define P2 2
#define DECK_SIZE 30 // Numero di carte nel deck
#define HAND_MAXSIZE 7 

int main(){

    GameState gamestate = GameState::Intro;

    // Definisci le dimensioni della finestra
    sf::Vector2u windowSize(1920, 1080);
    sf::RenderWindow window(sf::VideoMode(windowSize), "Progetto Tappa 05");

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

    // Creo le carte in mano, inizialmente zero
    std::vector<Card> cards;
    cards.reserve(10); 
    const int initialCard = 8;

    //Calcolo la dimensione della carta rendendola un po' più grande delle dimensioni degli slot
    float scaleFactor = 1.1f;
    sf::Vector2f cardSize(slotSize.x * scaleFactor, slotSize.y * scaleFactor);
    float spacing = 15.f;
    
    //Ripristino la posizione originale delle carte
    float y = windowSize.y - cardSize.y - 15.f; 

    // Funzione lambda per ricalcolare le posizioni delle carte in mano
    auto updateHandPositions = [&]() {
        if (cards.empty()) return;
        
        size_t numCards = cards.size();
        float currentCardWidth = cardSize.x;
        float currentSpacing = spacing;
        
        // Se abbiamo troppe carte, riduci la dimensione e la spaziatura
        if (numCards > HAND_MAXSIZE) {
            float scalingFactor = static_cast<float>(HAND_MAXSIZE) / numCards;
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
    }; 

    std::optional<size_t> selectedCardIndex; // Indice della carta selezionata
    float scrollOffset = 0.f; //Offset per lo scroll del testo dei dettagli della carta

    //Creo un clock per far passare qualche secondo prima di mostrare il campo di gioco
    sf::Clock clock;


    while(window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                std::cout << "Richiesta di chiusura..." << std::endl;
                window.close();
            }

            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                if(keyPressed->code == sf::Keyboard::Key::Enter && gamestate == GameState::Intro) {
                    std::cout << "Passaggio allo stato FieldVisible..." << std::endl;
                    gamestate = GameState::FieldVisible; 
                    clock.restart(); // 
                }
            }

            if(gamestate == GameState::FieldVisible && clock.getElapsedTime().asSeconds() > 1.f) {
                std::cout << "Passaggio allo stato Playing..." << std::endl;
                gamestate = GameState::Playing; // Passa allo stato di gioco dopo 1 secondo

                //Pesco le carte iniziali
                for(int i=0; i<initialCard; ++i){
                    if(!deck.isEmpty()){
                        Card drawnCard = deck.drawCard();
                        drawnCard.setTexture(textureNonFlipped);
                        drawnCard.setSize(cardSize); // Imposta la dimensione corretta per le carte in mano
                        
                        // Imposta il rettangolo di texture per mostrare l'intera texture
                        sf::Vector2u texSize = textureNonFlipped.getSize();
                        drawnCard.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texSize.x, texSize.y)));

                        cards.push_back(drawnCard);
                    }
                }
                
                // Ricalcola le posizioni dopo aver aggiunto le carte
                updateHandPositions();
            }

            if (const auto *mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Mouse::Button but = mouseButton->button;
                if(but == sf::Mouse::Button::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                    bool cardClicked = false;
                    for (size_t i = 0; i < cards.size(); ++i) {
                        if (cards[i].isClicked(mousePos)) {
                            // Se è una carta diversa da quella già selezionata, resetta lo scroll
                            if (selectedCardIndex.has_value() && selectedCardIndex.value() != i) {
                                scrollOffset = 0.f; // Reset dello scroll
                            }

                            selectedCardIndex = i; // Aggiorna l'indice della carta selezionata
                            cardClicked = true;
                            std::cout << "Hai cliccato: " << cards[i].getName() << std::endl;
                            break;
                        }
                    }

                     // Se ho cliccato fuori dalle carte, deseleziona
                    if (!cardClicked) {
                        selectedCardIndex.reset();
                        scrollOffset = 0.f; // Resettando lo scroll quando seleziono fuori (o un'altra carta) ritorno in cima alla view
                    }
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
        
        //Blocco Rendering
        window.clear(sf::Color::Black);

        // Ottieni la posizione del mouse
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        
        // Disegna sempre il campo di gioco (che include lo sfondo)
        field.draw(window, mousePos, gamestate);
        
        // Se siamo nello stato Intro, disegna il testo sopra lo sfondo
        if(gamestate == GameState::Intro) {
            drawStartScreen(window, detailFont, windowSize);
        } else {
            // Negli altri stati, disegna anche carte e deck
            for (auto& card : cards) card.draw(window);

            // Disegna il deck 
            deck.draw(window, mousePos, detailFont, deckSlotPos, slotSize, gamestate);

            // Se c'è una carta selezionata, mostra i dettagli
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