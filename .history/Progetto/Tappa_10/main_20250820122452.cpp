#include <SFML/Graphics.hpp>
#include "Field/Field.h"
#include "Deck/Deck.h"
#include "DrawAnimation/DrawAnimation.h"
#include "Deck/ShuffleAnimation.h"
#include "../resources/data/GameState.h"
#include "auxFunc.h"
#include "HomePage/HomePage.h"
#include "ResourceManager/ResourceManager.h"
#include "DeckSelectionScreen/DeckSelectionScreen.h"
#include "FieldLoadingAnimation/FieldLoadingAnimation.h"
#include "ExtraDeckOverlay/ExtraDeckOverlay.h"
#include "GameLogic/GameController.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <fstream>

#define P1 1 
#define P2 2
#define DECK_SIZE 30 // Numero di carte nel deck
#define HAND_MAXSIZE 7 
#define CARD_MAXOFFSET 50.f
#define MOVEMENT_SPEED 2000.f

// Path base per tutte le risorse
std::string resourceBase = "../../Progetto/resources/";
std::string texturePath = resourceBase + "textures/";
std::string deck1Path = texturePath + "textureDeck1/";
std::string fontPath = texturePath + "ITCKabelStdDemi.TTF";
std::string cardsJsonPath = resourceBase + "jsonData/cards.json";
std::string decksJsonPath = resourceBase + "jsonData/decks.json";


int main(){
    std::cout << "Programma avviato" << std::endl;

    std::vector<DrawAnimation> animations;
    int cardsToDraw = 0; // Numero di carte da pescare (coda)
    GameState gamestate = GameState::StartScreen;
    bool mousePressed = false; 
    bool showDrawnCardDetails = false;
    // Flag per l'animazione di shuffle (era static interno, lo rendiamo variabile per poterlo resettare)
    bool shuffleStarted = false;

    // Definisci le dimensioni della finestra
    sf::Vector2u windowSize(1920, 1080);
    sf::RenderWindow window(sf::VideoMode(windowSize), "Progetto Tappa 09");

    // TextureManager per tutte le texture
    TextureManager textureManager;

    sf::Texture& fieldTexture = textureManager.getTexture(texturePath + "backgroundTexture.png");
    std::cout << "Caricata: " << texturePath + "backgroundTexture.jpg" << std::endl;
    
    sf::Texture& monsterTexture = textureManager.getTexture(texturePath + "monsterText.png");
    std::cout << "Caricata: " << texturePath + "monsterText.png" << std::endl;
    
    sf::Texture& spellTrapTexture = textureManager.getTexture(texturePath + "SpellTrapTexture.png");
    std::cout << "Caricata: " << texturePath + "SpellTrapTexture.png" << std::endl;
    
    sf::Texture& deckTexture = textureManager.getTexture(texturePath + "deckTexture.png");
    std::cout << "Caricata: " << texturePath + "deckTexture.png" << std::endl;
    
    sf::Texture& graveyardTexture = textureManager.getTexture(texturePath + "graveTexture.png");
    std::cout << "Caricata: " << texturePath + "graveTexture.png" << std::endl;
    
    sf::Texture& extraDeckTexture = textureManager.getTexture(texturePath + "ExtraDeckTexture.png");
    std::cout << "Caricata: " << texturePath + "ExtraDeckTexture.png" << std::endl;
    
    sf::Texture& fieldSpellTexture = textureManager.getTexture(texturePath + "fieldSpell.png");
    std::cout << "Caricata: " << texturePath + "fieldSpell.png" << std::endl;

    sf::Texture& textureFlipped = textureManager.getTexture(texturePath + "Texture1.png");
    std::cout << "Caricata: " << texturePath + "Texture1.png" << std::endl;
    
    sf::Texture& textureNonFlipped = textureManager.getTexture(texturePath + "CardNotSet.jpg");
    std::cout << "Caricata: " << texturePath + "CardNotSet.jpg" << std::endl;

    sf::Texture& startScreenTexture = textureManager.getTexture(texturePath + "startScreenTexture.png");
    std::cout << "Caricata: " << texturePath + "startScreenTexture.png" << std::endl;

    sf::Texture& homeScreenTexture = textureManager.getTexture(texturePath + "homeScreenTexture.png");
    std::cout << "Caricata: " << texturePath + "homeScreenTexture.png" << std::endl;

    sf::Texture& deckSelectionScreenTexture = textureManager.getTexture(texturePath + "deckSelectionScreenTexture.png");
    std::cout << "Caricata: " << texturePath + "deckSelectionScreenTexture.png" << std::endl;

    // Carico il font per le etichette delle carte
    sf::Font detailFont;
    if (!detailFont.openFromFile(fontPath)) {
        std::cerr << "ERRORE: Impossibile caricare font: " << fontPath << std::endl;
        if (!detailFont.openFromFile("C:/Windows/Fonts/calibri.ttf")) {
            std::cerr << "ERRORE: Impossibile caricare nessun font!" << std::endl;
        } else {
            std::cout << "Caricato font di sistema: calibri.ttf" << std::endl;
        }
    } else {
        std::cout << "Caricato font: " << fontPath << std::endl;
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

    // Posizione degli slot del deck principale e dell'extra deck (giocatore 1)
    sf::Vector2f deckSlotPos = field.getSlotPosition(Type::Deck, P1); 
    sf::Vector2f extraDeckSlotPos = field.getSlotPosition(Type::Extra, P1); // usato per disegnare lo stack dell'extra deck


    // --- INTEGRAZIONE JSON ---
    // ========== CREA E INIZIALIZZA IL RESOURCE MANAGER ==========
    ResourceManager resourceManager(textureManager, resourceBase);

    if (!resourceManager.initialize(deckSlotPos, extraDeckSlotPos, deckCardSize, slotSize, textureFlipped)) {
        std::cerr << "ERRORE: Impossibile inizializzare ResourceManager!" << std::endl;
        std::cerr << "Il gioco continuerà con un deck vuoto." << std::endl;
    }
    
    Deck deck = resourceManager.getDeckByName(""); 
    
    // Overlay Extra Deck
    ExtraDeckOverlay extraOverlay(windowSize, slotSize, textureManager);

    // Creo le carte in mano, inizialmente il vettore raffigurante le carte in mano è vuoto
    std::vector<Card> cards;
    std::vector<Card> fieldCards; // Carte piazzate sul campo
    cards.reserve(10); 
    const int initialCard = 5;
    
    // Variabili per la gestione della selezione
    std::optional<size_t> selectedCardIndex;
    bool selectedCardIsOnField = false; // Traccia se la carta selezionata è sul campo

    //Calcolo la dimensione della carta rendendola un po' più grande delle dimensioni degli slot e prendo i parametri di posizione e spaziatura
    float scaleFactor = 1.1f;
    sf::Vector2f cardSize(slotSize.x * scaleFactor, slotSize.y * scaleFactor);
    float spacing = 15.f;
    float totalHandWidth = initialCard * cardSize.x + (initialCard - 1) * spacing;
    float startX = (windowSize.x - totalHandWidth) / 2.f;
    
    //Ripristino la posizione originale delle carte
    float y = windowSize.y - cardSize.y - 15.f; 

    float scrollOffset = 0.f; //Offset per lo scroll del testo dei dettagli della carta

    // Variabili per il dragging
    bool isDragging = false;
    bool isPotentialDrag = false; // Indica se il mouse è premuto su una carta ma non ancora in dragging
    std::optional<size_t> draggingCardIndex;
    std::optional<size_t> potentialDragCardIndex;
    sf::Vector2f dragOffset; // Offset tra mouse e posizione carta
    sf::Vector2f initialMousePos; // Posizione iniziale del mouse al click
    const float dragThreshold = 35.0f; // Soglia in pixel per attivare il dragging
    
    //Creazione della HomePage
    HomePage homeScreen(window, detailFont, windowSize, homeScreenTexture, deck.getDeckName());
    DeckSelectionScreen deckSelectionScreen(window, textureManager, resourceManager, deckSelectionScreenTexture, windowSize, detailFont);
    std::string selectedDeckName = "";
    bool deckSelected = false;

    // Funzione/lambda per resettare completamente la partita quando si torna alla Home
    auto resetMatch = [&](){
        // Cancella mano, carte sul campo e animazioni
        cards.clear();
        fieldCards.clear();
        animations.clear();
        cardsToDraw = 0;
        selectedCardIndex.reset();
        selectedCardIsOnField = false;
        isDragging = false;
        isPotentialDrag = false;
        draggingCardIndex.reset();
        potentialDragCardIndex.reset();
        showDrawnCardDetails = false;
        scrollOffset = 0.f;
        extraOverlay.handleEscape();
        shuffleStarted = false; // Permette di rieseguire la shuffle animation
        // Ricrea il deck (se è già stato scelto un nome) così le carte ritornano nel mazzo
        if(!selectedDeckName.empty()){
            deck = resourceManager.getDeckByName(selectedDeckName);
        } else {
            deck = resourceManager.getDeckByName("");
        }
        // Reimposta posizioni e stato fade del deck
        deck.resetDeckCardPositions(deckSlotPos, deckCardSize, slotSize, textureFlipped);
        deck.resetAnimation();
        // Prepara la coda pescate iniziali per la prossima partita (verrà attivata quando si entrerà in FieldLoading->FieldVisible->Playing)
        for(int i=0; i<initialCard && !deck.isEmpty(); ++i){
            ++cardsToDraw;
        }
    };
    
    // Avviso se si tenta di giocare senza aver selezionato un deck
    bool showNoDeckWarning = false;
    sf::Clock noDeckWarningClock;
    const float NO_DECK_WARNING_DURATION = 3.0f; // secondi
    sf::Text noDeckWarningText(detailFont, "Non si puo' giocare se non e' stato selezionato un Deck", 34);
    noDeckWarningText.setFillColor(sf::Color(255, 80, 80));
    noDeckWarningText.setStyle(sf::Text::Bold);
    noDeckWarningText.setOutlineColor(sf::Color(0,0,0,180));
    noDeckWarningText.setOutlineThickness(4.f);

    FieldLoadingAnimation fieldLoadingAnim = FieldLoadingAnimation(homeScreenTexture, fieldTexture, windowSize); // Animazione transizione Home->Field

    // Popup conferma ritorno alla Home
    bool returnPopupActive = false;

    //Creo il game Controller per gestire la logica dei turni
    GameController gameController = GameController(field, cards, fieldCards, deck, animations, cardsToDraw, slotSize);

    while(window.isOpen()){

        ///////////////////////////
        // Gestione Degli Eventi //
        ///////////////////////////

        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                std::cout << "Richiesta di chiusura..." << std::endl;
                window.close();
            }

            // Se il popup è attivo limitare la gestione eventi a popup stesso
            if(returnPopupActive){
                if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                    if(keyPressed->code == sf::Keyboard::Key::Y){
                        // Conferma ritorno
                        gamestate = GameState::HomeScreen;
                        returnPopupActive = false;
                        resetMatch();
                    } else if(keyPressed->code == sf::Keyboard::Key::N || keyPressed->code == sf::Keyboard::Key::Escape || keyPressed->code == sf::Keyboard::Key::Space){
                        returnPopupActive = false; // annulla
                    }
                }
                if(const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()){
                    if(mb->button == sf::Mouse::Button::Left){
                        sf::Vector2f center(windowSize.x/2.f, windowSize.y/2.f);
                        sf::Vector2f popupSize(600.f, 300.f);
                        sf::FloatRect popup(sf::Vector2f(center.x - popupSize.x/2.f, center.y - popupSize.y/2.f), popupSize);
                        sf::Vector2f btnSize(160.f,60.f);
                        float buttonY = popup.position.y + popup.size.y - btnSize.y - 40.f;
                        float gap = 80.f;
                        float totalBtnWidth = btnSize.x*2 + gap;
                        float startX = center.x - totalBtnWidth/2.f;
                        sf::FloatRect yesRect(sf::Vector2f(startX, buttonY), sf::Vector2f(btnSize.x, btnSize.y));
                        sf::FloatRect noRect(sf::Vector2f(startX + btnSize.x + gap, buttonY), sf::Vector2f(btnSize.x, btnSize.y));
                        sf::Vector2f mposF(static_cast<float>(sf::Mouse::getPosition(window).x), static_cast<float>(sf::Mouse::getPosition(window).y));
                        if(yesRect.contains(mposF)){
                            gamestate = GameState::HomeScreen;
                            returnPopupActive = false;
                            resetMatch();
                        } else if(noRect.contains(mposF)){
                            returnPopupActive = false;
                        }
                    }
                }
                // Salta altra gestione eventi mentre popup attivo
                continue;
            }

            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                // Invio dalla start screen
                if(keyPressed->code == sf::Keyboard::Key::Enter && gamestate == GameState::StartScreen) {
                    std::cout << "Passaggio allo stato HomeScreen..." << std::endl;
                    gamestate = GameState::HomeScreen; 
                }
                // Apertura popup conferma ritorno se stiamo giocando
                if(keyPressed->code == sf::Keyboard::Key::Space && gamestate == GameState::Playing){
                    returnPopupActive = true;
                }
                // ESC nella schermata di selezione deck: avvia fade out invece di cambio immediato
                if(keyPressed->code == sf::Keyboard::Key::Escape && gamestate == GameState::DeckSelection) {
                    std::cout << "ESC -> Fade out DeckSelection" << std::endl;
                    if(!deckSelectionScreen.isFading()) deckSelectionScreen.startFadeOut(0.4f);
                }
            }
            
            if (const auto *mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Mouse::Button but = mouseButton->button;
                // Tasto destro nella deck selection: fade out
                if(but == sf::Mouse::Button::Right && gamestate == GameState::DeckSelection) {
                    std::cout << "Right click -> Fade out DeckSelection" << std::endl;
                    if(!deckSelectionScreen.isFading()) deckSelectionScreen.startFadeOut(0.4f);
                }

                if(but == sf::Mouse::Button::Left) {
                    mousePressed = true;
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    initialMousePos = static_cast<sf::Vector2f>(mousePos);

                    // Avvio hold per overlay extra deck
                    extraOverlay.startHold(static_cast<sf::Vector2f>(mousePos), sf::FloatRect(extraDeckSlotPos, slotSize), gamestate);
                    // Gestione click su overlay (solo al momento del click, non ogni frame)
                    extraOverlay.handleMousePress(static_cast<sf::Vector2f>(mousePos), deck.getExtraCards());

                    if(gamestate == GameState::HomeScreen && homeScreen.getGiocaBounds().contains(initialMousePos)) {
                        if(selectedDeckName.empty()) {
                            std::cout << "Tentativo di giocare senza deck selezionato" << std::endl;
                            showNoDeckWarning = true;
                            noDeckWarningClock.restart();
                        } else {
                            std::cout << "Cliccato su Gioca!" << std::endl;
                            // Reset stato pescate (verranno avviate solo dopo Playing)
                            cardsToDraw = 0;
                            for(int i=0; i<initialCard && !deck.isEmpty(); ++i) ++cardsToDraw;
                            // Reset fade deck e animazioni (deckAlpha e flag)
                            deck.resetDeckCardPositions(deckSlotPos, deckCardSize, slotSize, textureFlipped);
                            deck.resetAnimation();
                            shuffleStarted = false; // forza nuova shuffle avanzata
                            
                            // Reset animazione del campo se ha uno stato interno (riavvia transizione FieldLoading->FieldVisible)
                            field.resetAnimation();
                            
                            // Impedisci che si entri direttamente in Playing cancellando eventuale shuffleAnimation terminata precedente
                            deck.clearShuffleAnimationAdvanced();
                            gamestate = GameState::FieldLoading; // Stato intermedio con animazione
                        }
                    } 

                    if(gamestate == GameState::HomeScreen && homeScreen.getSelectionBounds().contains(initialMousePos)) {
                        std::cout << "Cliccato su Selezione Mazzo!" << std::endl;
                        gamestate = GameState::DeckSelection;
                        deckSelectionScreen.startFadeIn(0.5f);
                    }

                    if(gamestate == GameState::DeckSelection){
                        if(deckSelectionScreen.getDeckBlueEyesBounds().contains(initialMousePos)) {
                            std::cout << "Selezionato mazzo Blue Eyes!" << std::endl;
                            selectedDeckName = "Blu-Eyes";
                            deckSelected = true;
                            deckSelectionScreen.setSelectedDeck(0);
                            homeScreen.setDeckName(selectedDeckName);
                            // Avvia fade out dopo selezione
                            if(!deckSelectionScreen.isFading()) deckSelectionScreen.startFadeOut(0.4f);
                        } else if(deckSelectionScreen.getDeckDarkMagicianBounds().contains(initialMousePos)) {
                            std::cout << "Selezionato mazzo Dark Magician!" << std::endl;
                            selectedDeckName = "Black-Magician";
                            deckSelected = true;
                            deckSelectionScreen.setSelectedDeck(1);
                            homeScreen.setDeckName(selectedDeckName);
                            if(!deckSelectionScreen.isFading()) deckSelectionScreen.startFadeOut(0.4f);
                        } else if(deckSelectionScreen.getDeckRedEyesBounds().contains(initialMousePos)) {
                            std::cout << "Selezionato mazzo Red Eyes!" << std::endl;
                            selectedDeckName = "Red-Eyes";
                            deckSelected = true;
                            deckSelectionScreen.setSelectedDeck(2);
                            homeScreen.setDeckName(selectedDeckName);
                            if(!deckSelectionScreen.isFading()) deckSelectionScreen.startFadeOut(0.4f);
                        }
                    }
                }
            }

            if(mousePressed && !isDragging) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                bool cardClicked = false;
                // Controlla prima le carte in mano per il drag
                for (size_t i = 0; i < cards.size(); ++i) {
                    if (cards[i].isClicked(mousePos)) {
                        isPotentialDrag = true;
                        potentialDragCardIndex = i;
                        sf::Vector2f cardPos = cards[i].getPosition();
                        dragOffset = static_cast<sf::Vector2f>(mousePos) - cardPos;
                        
                        if (!selectedCardIndex.has_value() || selectedCardIndex.value() != i) {
                            scrollOffset = 0.f;
                        }
                        selectedCardIndex = i;
                        selectedCardIsOnField = false; // Carta in mano
                        cardClicked = true;
                        break;
                    }
                }
                
                // Se non è stata cliccata una carta in mano, controlla le carte sul campo
                if (!cardClicked) {
                    for (size_t i = 0; i < fieldCards.size(); ++i) {
                        if (fieldCards[i].isClicked(mousePos)) {
                            cardClicked = true;
                            selectedCardIndex = i; 
                            selectedCardIsOnField = true; // Carta sul campo
                            scrollOffset = 0.f;
                            // Non impostare isPotentialDrag per le carte sul campo
                            break;
                        }
                    }
                }
                
                if (!cardClicked) {
                    selectedCardIndex.reset();
                    selectedCardIsOnField = false;
                    scrollOffset = 0.f;

                } 
            }

            // Mouse released: gestisce drop zones
            if (const auto *mouseButton = event->getIf<sf::Event::MouseButtonReleased>()) {
                sf::Mouse::Button but = mouseButton->button;
                if(but == sf::Mouse::Button::Left) {
                    extraOverlay.handleMouseRelease();
                    
                    if (isDragging && draggingCardIndex.has_value()) {
                        
                        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                        
                        if (findSlotPosition(mousePos, cards[draggingCardIndex.value()], field, slotSize).has_value()) {
                            // Posizione valida: sposta la carta al centro dello slot e marca lo slot come occupato
                            sf::Vector2f slotPos = findSlotPosition(mousePos, cards[draggingCardIndex.value()], field, slotSize).value();
                            
                            // Sposta la carta dal vettore cards al vettore fieldCards
                            Card cardToField = cards[draggingCardIndex.value()];
                            
                            // Ridimensiona la carta per il campo (85% della dimensione dello slot)
                            float fieldCardScale = 0.85f;
                            sf::Vector2f fieldCardSize(slotSize.x * fieldCardScale, slotSize.y * fieldCardScale);
                            cardToField.setSize(fieldCardSize);
                            
                            // Centra la carta nello slot
                            sf::Vector2f centeredPos;
                            centeredPos.x = slotPos.x + (slotSize.x - fieldCardSize.x) / 2.0f;
                            centeredPos.y = slotPos.y + (slotSize.y - fieldCardSize.y) / 2.0f;
                            cardToField.setPosition(centeredPos);
                            
                            fieldCards.push_back(cardToField);
                            
                            // Rimuovi la carta dal vettore cards
                            cards.erase(cards.begin() + draggingCardIndex.value());
                            
                            // Ottieni lo slot corrispondente e marcalo come occupato
                            Slot* targetSlot = field.getSlotByPosition(slotPos);
                            if (targetSlot != nullptr) {
                                targetSlot->setOccupied(true);
                                std::cout << "Carta piazzata nello slot e slot marcato come occupato!" << std::endl;
                            } else {
                                std::cout << "Carta piazzata nello slot!" << std::endl;
                            }
                            
                            // Riorganizza le carte rimaste in mano
                            updateHandPositions(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                        
                        } else {
                            // Posizione non valida: calcola la posizione originale nella mano e riposiziona
                            Card tempCard = cards[draggingCardIndex.value()];
                            cards.erase(cards.begin() + draggingCardIndex.value());
                            sf::Vector2f originalPos = setHandPos(cards, tempCard, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                            cards.insert(cards.begin() + draggingCardIndex.value(), tempCard);
                            cards[draggingCardIndex.value()].setPosition(originalPos);
                            updateHandPositions(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                            std::cout << "Posizione non valida! Carta torna in mano." << std::endl;
                        }
                    }
                    
                    mousePressed = false;
                    isDragging = false;
                    isPotentialDrag = false;
                    draggingCardIndex.reset();
                    potentialDragCardIndex.reset();
                }
            }

            // Gestione tasto ESC per deselezionare
            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed->code == sf::Keyboard::Key::Escape) {
                        // Se non siamo in DeckSelection gestiamo la logica di deselezione carte
                        if(gamestate != GameState::DeckSelection) {
                            selectedCardIndex.reset();
                            scrollOffset = 0.f; // Resetta lo scroll anche con ESC
                            std::cout << "Carta deselezionata con ESC" << std::endl;
                            // Chiudi anche overlay extra deck
                            extraOverlay.handleEscape();
                        }
                    }
                }
             
            // Scroll mouse
            if (const auto* mouseScroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                // Permetti lo scroll sia se una carta è selezionata, sia se si sta mostrando la carta pescata
                if (selectedCardIndex.has_value() || showDrawnCardDetails || extraOverlay.detailsActive()) {
                    scrollOffset -= mouseScroll->delta * 20.f;
                    scrollOffset = std::max(0.f, scrollOffset); // niente scroll negativo
                }
            }
        }

        /////////////////////////////////
        // Update dello stato del gioco//
        /////////////////////////////////
        
        homeScreen.update();
        deckSelectionScreen.update(window);
        static sf::Clock clock;
        float deltaTime = clock.restart().asSeconds();
        // Pausa logica/animazioni se popup attivo (deltaTime annullato)
        if(returnPopupActive) deltaTime = 0.f;
        
        // Se è stato selezionato un deck, caricalo e re-inizializza lo stato della mano/pescate
        if(deckSelected){
            deck = resourceManager.getDeckByName(selectedDeckName);
            // Reimposta stato di gioco relativo alla mano e alle animazioni di pescata
            cards.clear();
            fieldCards.clear();
            animations.clear();
            selectedCardIndex.reset();
            selectedCardIsOnField = false;
            cardsToDraw = 0;
            for(int i=0; i<initialCard && !deck.isEmpty(); ++i){
                ++cardsToDraw;
            }
            // Assicura che il nome sia aggiornato (già fatto nell'handler ma nel dubbio)
            homeScreen.setDeckName(selectedDeckName);
            deckSelected = false; // Evita di rieseguire ogni frame
        }

        // Gestione animazione FieldLoading
        if(gamestate == GameState::FieldLoading && !fieldLoadingAnim.hasStarted()){
            fieldLoadingAnim.start(homeScreenTexture, fieldTexture, windowSize);
        }
        if(gamestate == GameState::FieldLoading){
            fieldLoadingAnim.update(deltaTime);
            if(fieldLoadingAnim.isFinished()){
                std::cout << "Transizione completata: FieldLoading -> FieldVisible" << std::endl;
                gamestate = GameState::FieldVisible;
            }
        }

        // Controlla se il mouse si è mosso abbastanza e nella direzione corretta per attivare il dragging
        if (isPotentialDrag && potentialDragCardIndex.has_value()) {
            sf::Vector2f currentMousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
            float distance = std::sqrt(std::pow(currentMousePos.x - initialMousePos.x, 2) + std::pow(currentMousePos.y - initialMousePos.y, 2));
            
            // Calcola il movimento verticale (negativo = verso l'alto)
            float verticalMovement = currentMousePos.y - initialMousePos.y;
            bool isMovingTowardsField = verticalMovement < -static_cast<float>(dragThreshold); // Movimento verso l'alto per P1
            
            if (distance >= dragThreshold && isMovingTowardsField) {
                // Attiva il dragging solo se il movimento è verso il campo
                isDragging = true;
                draggingCardIndex = potentialDragCardIndex;
                isPotentialDrag = false;
                potentialDragCardIndex.reset();
                // Deseleziona la carta quando inizia il dragging per nascondere i dettagli
                selectedCardIndex.reset();
                scrollOffset = 0.f;
            }
        }

        if(gamestate == GameState::FieldVisible) {
            field.animate(deltaTime);
            if(field.isAnimationFinished()) {
                deck.animate(deltaTime);
                if(deck.isAnimationFinished()) {
                    deck.setAnimationFinished();
                    // Avvia shuffle animation avanzata una sola volta
                    // Avvia shuffle animation avanzata una sola volta (resetMatch la riporta a false)
                    if (!shuffleStarted) {
                        deck.startShuffleAnimationAdvanced(deckSlotPos, deckCardSize);
                        shuffleStarted = true;
                    }
                    // Aggiorna shuffle animation
                    deck.updateShuffleAnimationAdvanced(deltaTime);
                    // Quando l'animazione è finita, mischia effettivamente il deck e passa allo stato Playing
                    if (deck.isShuffleAnimationAdvancedFinished()) {
                        deck.shuffle();
                        deck.resetDeckCardPositions(deckSlotPos, deckCardSize, slotSize, textureFlipped);
                        std::cout << "Passaggio allo stato Playing..." << std::endl;
                        gamestate = GameState::Playing;
                        game
                    }
                }
            }
        }

        // Avvia la prima animazione di pescata SOLO dopo il passaggio allo stato Playing
        if (!returnPopupActive && gamestate == GameState::Playing && cardsToDraw > 0 && animations.empty() && !deck.isEmpty()) {
            Card nextCard = deck.drawCard();
            --cardsToDraw;
            DrawAnimation anim(
                nextCard, DrawAnimationPhases::MovingOut, deckSlotPos,
                sf::Vector2f(windowSize.x / 2.f - cardSize.x / 2.f, windowSize.y / 2.f - cardSize.y / 2.f),
                textureManager
            );
            animations.push_back(anim);
        }

        Card tmpcard = Card("", "", 0, 0, sf::Vector2f(0.f, 0.f), sf::Vector2f(0.f, 0.f), textureFlipped, Type::Monster, Attribute::None, 0, {});

        if(gamestate == GameState::Playing && !returnPopupActive){
            // Aggiorna solo la prima animazione (una alla volta)
            if (!animations.empty()) {
                DrawAnimationPhases actualPhase;
                // Passa mousePressed come skipPause
                actualPhase = animations.front().update(MOVEMENT_SPEED, deltaTime, textureNonFlipped, cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE, mousePressed, textureManager);
                if(actualPhase == DrawAnimationPhases::ShowCard){
                    showDrawnCardDetails = true;
                    tmpcard = animations.front().getCard();
                }
                if (actualPhase == DrawAnimationPhases::Done) {
                    showDrawnCardDetails = false;
                    cards.push_back(animations.front().getCard());
                    animations.erase(animations.begin());
                    updateHandPositions(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                    deck.resetDeckCardPositions(deckSlotPos, deckCardSize, slotSize, textureFlipped);
                    // Avvia la prossima animazione se ci sono altre carte da pescare
                    if (cardsToDraw > 0 && !deck.isEmpty()) {
                        Card nextCard = deck.drawCard();
                        --cardsToDraw;
                        DrawAnimation anim(
                            nextCard, DrawAnimationPhases::MovingOut, deckSlotPos,
                               sf::Vector2f(windowSize.x / 2.f - cardSize.x / 2.f, windowSize.y / 2.f - cardSize.y / 2.f),
                               textureManager
                        );
                        animations.push_back(anim);
                    }
                }
            }
        }

        // Gestione hold extra deck (attiva preview dopo soglia se non sto trascinando carte)
        if(!returnPopupActive)
            extraOverlay.update(deltaTime, isDragging, gamestate);

        //Gestione del sollevamento delle carte in mano
        float targetCardOffset = 0.f;
        float offsetSpeed = 200.f; 
        if(!returnPopupActive){
            for(size_t i = 0; i<cards.size(); ++i){
                float currentOffset = cards[i].getOffset();
                if(selectedCardIndex.has_value() && !selectedCardIsOnField && selectedCardIndex.value() == i) {
                    targetCardOffset = CARD_MAXOFFSET;
                } else {
                    targetCardOffset = 0.f; 
                }
                if(currentOffset < targetCardOffset) {
                    currentOffset = std::min(currentOffset + offsetSpeed * deltaTime, targetCardOffset); 
                    cards[i].setOffset(currentOffset);
                } else {
                    currentOffset = std::max(currentOffset - offsetSpeed * deltaTime, targetCardOffset);
                    cards[i].setOffset(currentOffset);
                }    
            }
        }

        ///////////////
        // Rendering //
        ///////////////

        window.clear(sf::Color::Black);

        // Ottieni la posizione del mouse
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        // Aggiorna la posizione della carta durante il dragging
        if (isDragging && draggingCardIndex.has_value() && draggingCardIndex.value() < cards.size()) {
            sf::Vector2f newCardPos = static_cast<sf::Vector2f>(mousePos) - dragOffset;
            cards[draggingCardIndex.value()].setPosition(newCardPos);
        }

        // Se siamo nello stato StartScreen, disegna il testo sopra lo sfondo con animazione fade
        static sf::Clock startScreenClock;
        float startScreenElapsed = startScreenClock.getElapsedTime().asSeconds();
        if(gamestate == GameState::StartScreen) drawStartScreen(window, detailFont, windowSize, startScreenTexture, startScreenElapsed);

        if(gamestate == GameState::HomeScreen){
            
            homeScreen.draw(window);
            if(showNoDeckWarning){
                float elapsed = noDeckWarningClock.getElapsedTime().asSeconds();
                if(elapsed < NO_DECK_WARNING_DURATION){
                    sf::FloatRect bounds = noDeckWarningText.getLocalBounds();
                    noDeckWarningText.setPosition(sf::Vector2f((windowSize.x - bounds.size.x)/2.f, windowSize.y * 0.78f));
                    // Effetto fade-out negli ultimi 0.8 secondi
                    float alpha = 255.f;
                    float fadeTime = 0.8f;
                    if(NO_DECK_WARNING_DURATION - elapsed < fadeTime){
                        alpha = 255.f * (NO_DECK_WARNING_DURATION - elapsed) / fadeTime;
                    }
                    sf::Color color = noDeckWarningText.getFillColor();
                    color.a = static_cast<uint8_t>(std::max(0.f, std::min(255.f, alpha)));
                    noDeckWarningText.setFillColor(color);
                    window.draw(noDeckWarningText);
                } else {
                    showNoDeckWarning = false;
                }
            }
        }   
        
        else if(gamestate == GameState::FieldLoading){
            // Disegna solo l'animazione di transizione
            fieldLoadingAnim.draw(window);
            window.display();
            continue;
        } else  {
            // Disegna il campo di gioco
            if(gamestate == GameState::FieldVisible || gamestate == GameState::Playing ) {
                bool enableHover = !extraOverlay.isOverlayVisible() && !returnPopupActive;
                field.draw(window, mousePos, gamestate, enableHover);
            }

            // Disegna il deck principale (o la shuffle animation) e l'extra deck sopra il relativo slot
            if(field.isAnimationFinished()) {
                if (deck.getSize() > 0) {
                    if (!deck.isShuffleAnimationAdvancedFinished()) {
                        deck.drawShuffleAnimationAdvanced(window);
                    } else {
                        deck.draw(window, mousePos, detailFont, deckSlotPos, slotSize, gamestate);
                    }
                }
                // Extra deck: disegna sempre lo stack se ha carte
                if (deck.getExtraSize() > 0) {
                    deck.drawExtra(window, detailFont, extraDeckSlotPos, slotSize, gamestate);
                }
            }

            if (showDrawnCardDetails) {
                sf::Vector2f panelPos{400.f, 150.f};
                sf::Vector2f panelSize{300.f, 200.f};
                showCardDetails(window, tmpcard, detailFont, panelPos, panelSize, scrollOffset);
            }

            //Disegna le animazioni delle carte
            if(deck.isAnimationFinished()) for(auto& anim : animations) anim.draw(window);

            // Disegna le carte sul campo
            for (auto& fieldCard : fieldCards) {
                fieldCard.draw(window);
            }

            // Disegna le carte in mano
            for (size_t i = 0; i < cards.size(); ++i) {
                sf::Vector2f originalPos = cards[i].getPosition();
                float offset = cards[i].getOffset();
                cards[i].setPosition(originalPos - sf::Vector2f(0.f, offset));
                cards[i].draw(window);
                cards[i].setPosition(originalPos);
            }

            // Se c'è una carta selezionata, mostra i dettagli - Non influisce sullo stato del gioco, ma solo sulla visualizzazione (rendering)
            if (selectedCardIndex.has_value()) {
                const Card* selectedCard = nullptr;
                
                // Determina quale carta è selezionata (in mano o sul campo)
                if (selectedCardIsOnField && selectedCardIndex.value() < fieldCards.size()) {
                    selectedCard = &fieldCards[selectedCardIndex.value()];
                } else if (!selectedCardIsOnField && selectedCardIndex.value() < cards.size()) {
                    selectedCard = &cards[selectedCardIndex.value()];
                }
                
                // Mostra i dettagli solo se abbiamo una carta valida
                if (selectedCard != nullptr) {
                    sf::Vector2f panelPos{400.f, 150.f};
                    sf::Vector2f panelSize{300.f, 200.f};
                    showCardDetails(window, *selectedCard, detailFont, panelPos, panelSize, scrollOffset);
                }
            }

            // Overlay preview extra deck (mostra tutte le carte) - sopra tutto il resto ma sotto eventuali pannelli dettagli
            // Overlay extra deck
            extraOverlay.draw(window, detailFont, deck.getExtraCards(), scrollOffset);
            if(extraOverlay.detailsActive()) {
                if(const Card* c = extraOverlay.getSelectedCard()){
                    sf::Vector2f panelPos{ windowSize.x * 0.07f, windowSize.y * 0.12f };
                    sf::Vector2f panelSize{ 340.f, 260.f };
                    showCardDetails(window, *c, detailFont, panelPos, panelSize, scrollOffset);
                }
            }
        }

        if (gamestate == GameState::DeckSelection) {
            deckSelectionScreen.draw(window);
            if(deckSelectionScreen.isFadeOutFinished()) {
                gamestate = GameState::HomeScreen;
            }
            window.display();
            continue;
        }

        // Disegno popup conferma se attivo (sopra tutto prima del display finale)
        if(returnPopupActive){
            sf::RectangleShape overlay(sf::Vector2f(windowSize.x, windowSize.y));
            overlay.setFillColor(sf::Color(0,0,0,140));
            window.draw(overlay);
            sf::Vector2f popupSize(600.f,300.f);
            sf::Vector2f center(windowSize.x/2.f, windowSize.y/2.f);
            sf::Vector2f popupPos(center.x - popupSize.x/2.f, center.y - popupSize.y/2.f);
            sf::RectangleShape panel(popupSize);
            panel.setPosition(popupPos);
            panel.setFillColor(sf::Color(25,25,25,235));
            panel.setOutlineColor(sf::Color(255,200,90));
            panel.setOutlineThickness(3.f);
            window.draw(panel);

            sf::Text title(detailFont, "Torna alla Home?", 48);
            title.setFillColor(sf::Color(255,230,140));
            title.setOutlineColor(sf::Color(0,0,0,200));
            title.setOutlineThickness(3.f);
            sf::FloatRect titleBounds = title.getLocalBounds();
            title.setOrigin(sf::Vector2f(titleBounds.size.x/2.f, titleBounds.size.y/2.f));
            title.setPosition(sf::Vector2f(center.x, popupPos.y + 70.f));
            window.draw(title);

            // Bottoni grafici
            sf::Vector2f btnSize(160.f,60.f);
            float gap = 80.f;
            float totalBtnWidth = btnSize.x*2 + gap;
            float startX = center.x - totalBtnWidth/2.f;
            float bottomMargin = 60.f;
            float buttonY = popupPos.y + popupSize.y - btnSize.y - bottomMargin;
            sf::RectangleShape yesButton(btnSize); 
            yesButton.setPosition(sf::Vector2f(startX, buttonY));
            sf::RectangleShape noButton(btnSize); 
            noButton.setPosition(sf::Vector2f(startX + btnSize.x + gap, buttonY));

            // Hover detection
            sf::Vector2f mouseF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
            bool hoverYes = yesButton.getGlobalBounds().contains(mouseF);
            bool hoverNo  = noButton.getGlobalBounds().contains(mouseF);

            //Colore base dei tasti
            auto baseYes = sf::Color(60,160,60);
            auto baseNo  = sf::Color(170,60,60);

            //Funzione Lambda che prende un colore e ritorna il colore schiarito
            auto lighten = [](sf::Color c){
                return sf::Color(
                    static_cast<uint8_t>(std::min(255, int(c.r*1.25f))),
                    static_cast<uint8_t>(std::min(255, int(c.g*1.25f))),
                    static_cast<uint8_t>(std::min(255, int(c.b*1.25f))),
                    c.a
                );
            };
            
            yesButton.setFillColor(hoverYes ? lighten(baseYes) : baseYes);
            noButton.setFillColor(hoverNo ? lighten(baseNo) : baseNo);
            yesButton.setOutlineThickness(hoverYes ? 4.f : 2.f);
            noButton.setOutlineThickness(hoverNo ? 4.f : 2.f);
            yesButton.setOutlineColor(sf::Color(0,0,0,180));
            noButton.setOutlineColor(sf::Color(0,0,0,180));
            window.draw(yesButton); 
            window.draw(noButton);

            // Testo centrato considerando anche offset (position) del localBounds
            sf::Text yesText(detailFont, "SI", 30); 
            sf::FloatRect yesBounds = yesText.getLocalBounds();
            yesText.setOrigin(sf::Vector2f(yesBounds.position.x + yesBounds.size.x/2.f, yesBounds.position.y + yesBounds.size.y/2.f));
            yesText.setPosition(yesButton.getPosition() + sf::Vector2f(btnSize.x/2.f, btnSize.y/2.f));
            sf::Text noText(detailFont, "NO", 30); 
            sf::FloatRect noBounds = noText.getLocalBounds(); 
            noText.setOrigin(sf::Vector2f(noBounds.position.x + noBounds.size.x/2.f, noBounds.position.y + noBounds.size.y/2.f)); 
            noText.setPosition(noButton.getPosition() + sf::Vector2f(btnSize.x/2.f, btnSize.y/2.f)); 
            window.draw(yesText);
            window.draw(noText);
        }

        window.display();
    }
    return 0;
}