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
#include "GraveyardOverlay/GraveyardOverlay.h"
#include "GameLogic/Game/Game.h"
#include "GameLogic/GamePhases.h"
#include "GameLogic/DrawController/DrawController.h"
#include "GameLogic/DiscardController/DiscardController.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <fstream>
#include <memory>
#include "GameLogic/Game/Effects/cards/WhiteStoneLegendEffect.h"
#include "GameLogic/Game/Effects/cards/WhiteStoneAncientsEffect.h"

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

    // DrawController centralizza tutte le pescate (rimossi counters legacy)
    GameState gamestate = GameState::StartScreen;
    bool mousePressed = false; 
    // Stato selezione tributi
    bool selectingTributes = false;
    bool chooseSummonOrSet = false; // Mostro: quando droppo su slot valido
    // Magie/Trappole: quando droppo su slot valido -> scelta Attiva/Setta
    bool chooseActivateOrSetST = false;
    std::optional<size_t> pendingHandIndexForPlay; // carta della mano droppata
    bool pendingIsSpellTrap = false; // true se la pending riguarda M/T
    std::vector<size_t> selectedTributes; // indici monster zone corrente
    int tributesNeeded = 0;
    bool showDrawnCardDetails = false;

    // Battle Phase state (v1)
    bool attackSelectionActive = false;
    std::optional<size_t> selectedAttackerIndex;
    std::string battleFeedbackMsg;
    sf::Clock battleFeedbackClock;
    const float BATTLE_FEEDBACK_DURATION = 2.0f; 

    // Flag per l'animazione di shuffle (era static interno, lo rendiamo variabile per poterlo resettare)
    bool shuffleStarted = false;

    // Definisci le dimensioni della finestra
    sf::Vector2u windowSize(1920, 1080);
    sf::RenderWindow window(sf::VideoMode(windowSize), "Progetto Tappa 12");

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
    GraveyardOverlay graveyardOverlay(windowSize, slotSize, textureManager);

    // Creo le carte in mano, inizialmente il vettore raffigurante le carte in mano è vuoto
    std::vector<Card>* handPtr = nullptr; // riferimento alla mano del Player corrente (Player.hand)
    std::vector<Card> fieldCards; // Carte piazzate sul campo (giocatore corrente)
    std::vector<Card> oppFieldCards; // Carte avversario renderizzate
    // Zone M/T e Campo
    std::vector<Card> p1STCards; // P1 = giocatore assoluto 0 (in basso)
    std::vector<Card> p2STCards; // P2 = giocatore assoluto 1 (in alto)
    std::optional<Card> p1FieldSpellCard;
    std::optional<Card> p2FieldSpellCard;
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

    // Logica di gioco principale (creata dopo la selezione del deck)
    std::unique_ptr<Game> game; // gestisce giocatori, turni, fasi
    DrawController drawController; 
    DiscardController discardController; 
    // pending auto end-turn ora gestito dentro Game

    int drawStartSubscriptionId = -1;   // sottoscrizione evento DrawStart
    int drawEndSubscriptionId = -1;     // sottoscrizione evento DrawEnd
    int endPhaseDiscardSubscriptionId = -1; // scarto automatico in End Phase
    // Stato overlay scelta per Evocazione Speciale (Attacco/Difesa)
    bool ssChoiceActive = false;
    int ssChoiceOwner = -1; // info minima
    int attackDeclaredSubId = -1;
    int attackResolvedSubId = -1;
    int lifeChangedSubId = -1;
    int monsterDestroyedSubId = -1;
    int directAttackSubId = -1;
    // Finestra semplice di risposta per Trappole/Rapide nel turno avversario
    bool responseWindowActive = false;
    
    // Sincronizza la rappresentazione grafica della zona mostri (Player1) con lo stato logico del Game
    auto syncMonsterZoneToField = [&](){
        if(!game) return;
    // In basso (P1) deve sempre mostrare la zona del Giocatore 0, in alto (P2) quella del Giocatore 1,
    // anche quando è il turno del Giocatore 1. Poiché Game espone solo le viste current/opponent,
    // ricaviamo le zone assolute dall'indice del giocatore corrente.
        int cur = game->getTurn().getCurrentPlayerIndex();
        const auto &p1Zone = (cur == 0) ? game->getMonsterZone() : game->getOpponentMonsterZone();
        const auto &p2Zone = (cur == 0) ? game->getOpponentMonsterZone() : game->getMonsterZone();

        fieldCards.clear();
        float fieldCardScale = 0.75f;
        sf::Vector2f fieldCardSize(slotSize.x * fieldCardScale, slotSize.y * fieldCardScale);
        for(size_t i=0;i<p1Zone.size();++i){
            Card c = p1Zone[i]; // copia per il rendering
            // Se la carta è scoperta sul campo P1, ripristina la texture originale frontale
            bool isFD = game->isFaceDownAt(0, i);
            if(!isFD && !c.originalTexturePath.empty()){
                sf::Texture &front = textureManager.getTexture(c.originalTexturePath);
                c.setTexture(front);
                sf::Vector2u ts = front.getSize();
                c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i((int)ts.x, (int)ts.y)));
            }
            sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, P1, static_cast<int>(i));
            sf::Vector2f centeredPos(slotPos.x + (slotSize.x - fieldCardSize.x)/2.f,
                                     slotPos.y + (slotSize.y - fieldCardSize.y)/2.f);
            c.setSize(fieldCardSize);
            c.setPosition(centeredPos);
            fieldCards.push_back(c);
        }
    // Avversario (P2 in alto)
        oppFieldCards.clear();
        for(size_t i=0;i<p2Zone.size();++i){
            Card c = p2Zone[i];
            // Il lato alto è il Giocatore 1: se scoperta, usa texture originale
            bool isFD2 = game->isFaceDownAt(1, i);
            if(!isFD2 && !c.originalTexturePath.empty()){
                sf::Texture &front = textureManager.getTexture(c.originalTexturePath);
                c.setTexture(front);
                sf::Vector2u ts = front.getSize();
                c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i((int)ts.x, (int)ts.y)));
            }
            sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, P2, static_cast<int>(i));
            sf::Vector2f centeredPos(slotPos.x + (slotSize.x - fieldCardSize.x)/2.f,
                                     slotPos.y + (slotSize.y - fieldCardSize.y)/2.f);
            c.setSize(fieldCardSize);
            c.setPosition(centeredPos);
            oppFieldCards.push_back(c);
        }
    // Corregge un eventuale indice selezionato non più valido
        if(selectedCardIsOnField && selectedCardIndex.has_value() && selectedCardIndex.value() >= fieldCards.size()){
            selectedCardIndex.reset();
            selectedCardIsOnField = false;
        }
    };
    // Sincronizza M/T e Field Spell con lo stato logico
    auto syncSpellTrapZones = [&](){
        if(!game) return;
        int cur = game->getTurn().getCurrentPlayerIndex();
        const auto &p1ST = (cur==0) ? game->getSpellTrapZone() : game->getOpponentSpellTrapZone();
        const auto &p2ST = (cur==0) ? game->getOpponentSpellTrapZone() : game->getSpellTrapZone();

        p1STCards.clear(); p2STCards.clear();
        // P1 in basso = assoluto 0
        for(size_t i=0;i<p1ST.size();++i){
            Card c = p1ST[i];
            bool isFD = game->isSpellTrapFaceDownAt(0, i);
            if(!isFD && !c.originalTexturePath.empty()){
                sf::Texture &front = textureManager.getTexture(c.originalTexturePath);
                c.setTexture(front);
                sf::Vector2u ts = front.getSize();
                c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i((int)ts.x, (int)ts.y)));
            }
            sf::Vector2f slotPos = field.getSlotPosition(Type::SpellTrap, P1, (int)i);
            sf::Vector2f size(slotSize.x*0.70f, slotSize.y*0.70f);
            c.setSize(size);
            c.setPosition(slotPos + sf::Vector2f((slotSize.x-size.x)/2.f, (slotSize.y-size.y)/2.f));
            p1STCards.push_back(c);
        }
        // P2 in alto = assoluto 1
        for(size_t i=0;i<p2ST.size();++i){
            Card c = p2ST[i];
            bool isFD = game->isSpellTrapFaceDownAt(1, i);
            if(!isFD && !c.originalTexturePath.empty()){
                sf::Texture &front = textureManager.getTexture(c.originalTexturePath);
                c.setTexture(front);
                sf::Vector2u ts = front.getSize();
                c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i((int)ts.x, (int)ts.y)));
            }
            sf::Vector2f slotPos = field.getSlotPosition(Type::SpellTrap, P2, (int)i);
            sf::Vector2f size(slotSize.x*0.70f, slotSize.y*0.70f);
            c.setSize(size);
            c.setPosition(slotPos + sf::Vector2f((slotSize.x-size.x)/2.f, (slotSize.y-size.y)/2.f));
            p2STCards.push_back(c);
        }
        // Field Spell slot singolo per lato
        p1FieldSpellCard = game->getFieldSpellOf(0);
        if(p1FieldSpellCard.has_value()){
            Card &c = p1FieldSpellCard.value();
            if(!c.originalTexturePath.empty()){
                sf::Texture &front = textureManager.getTexture(c.originalTexturePath);
                c.setTexture(front);
                sf::Vector2u ts = front.getSize();
                c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i((int)ts.x, (int)ts.y)));
            }
            sf::Vector2f pos = field.getSlotPosition(Type::FieldSpell, P1);
            sf::Vector2f size(slotSize.x*0.78f, slotSize.y*0.78f);
            c.setSize(size);
            c.setPosition(pos + sf::Vector2f((slotSize.x-size.x)/2.f, (slotSize.y-size.y)/2.f));
        }
        p2FieldSpellCard = game->getFieldSpellOf(1);
        if(p2FieldSpellCard.has_value()){
            Card &c = p2FieldSpellCard.value();
            if(!c.originalTexturePath.empty()){
                sf::Texture &front = textureManager.getTexture(c.originalTexturePath);
                c.setTexture(front);
                sf::Vector2u ts = front.getSize();
                c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i((int)ts.x, (int)ts.y)));
            }
            sf::Vector2f pos = field.getSlotPosition(Type::FieldSpell, P2);
            sf::Vector2f size(slotSize.x*0.78f, slotSize.y*0.78f);
            c.setSize(size);
            c.setPosition(pos + sf::Vector2f((slotSize.x-size.x)/2.f, (slotSize.y-size.y)/2.f));
        }
    };
    

    // Funzione/lambda per resettare completamente la partita quando si torna alla Home
    // Cancella mano, carte sul campo e animazioni
    auto resetMatch = [&](){
        if(handPtr) handPtr->clear();
        fieldCards.clear();
        oppFieldCards.clear();
        drawController.reset();
        selectedCardIndex.reset();
        selectedCardIsOnField = false;
        isDragging = false;
        isPotentialDrag = false;
        draggingCardIndex.reset();
        potentialDragCardIndex.reset();
        showDrawnCardDetails = false;
        scrollOffset = 0.f;
        extraOverlay.handleEscape();
        graveyardOverlay.handleEscape();
        shuffleStarted = false; // Permette di rieseguire la shuffle animation
        // Reset battle UI
        attackSelectionActive = false;
        selectedAttackerIndex.reset();
        battleFeedbackMsg.clear();
        
        // Reset logica partita
        game.reset();
        handPtr = nullptr;
        // Ricrea il deck (se è già stato scelto un nome) così le carte ritornano nel mazzo
        if(!selectedDeckName.empty()){
            deck = resourceManager.getDeckByName(selectedDeckName);
        } else {
            deck = resourceManager.getDeckByName("");
        }
        // Reimposta posizioni e stato fade del deck
        deck.resetDeckCardPositions(deckSlotPos, deckCardSize, slotSize, textureFlipped);
        deck.resetAnimation();
        
    };

    // Helper per ripristinare la posizione di una carta della mano all'indice dato
    auto restoreCardAtHandIndex = [&](size_t idx){
        if(!handPtr) return;
        if(idx >= handPtr->size()) return;
        Card tempCard = (*handPtr)[idx];
        handPtr->erase(handPtr->begin() + idx);
        sf::Vector2f originalPos = setHandPos(*handPtr, tempCard, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
        handPtr->insert(handPtr->begin() + idx, tempCard);
        (*handPtr)[idx].setPosition(originalPos);
        updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
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
    // Overlay Game Over (vittoria/sconfitta)
    bool gameOverActive = false;

    while(window.isOpen()){

        ///////////////////////////
        // Gestione Degli Eventi //
        ///////////////////////////

        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                std::cout << "Richiesta di chiusura..." << std::endl;
                window.close();
            }

            // Se è attivo il Game Over, gestisci solo i tasti per tornare alla Home e ignora il resto
            if(gameOverActive){
                if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                    if(keyPressed->code == sf::Keyboard::Key::Space || keyPressed->code == sf::Keyboard::Key::Enter || keyPressed->code == sf::Keyboard::Key::Escape){
                        gamestate = GameState::HomeScreen;
                        gameOverActive = false;
                        resetMatch();
                    }
                }
                // salta ulteriore gestione eventi in stato game over
                continue;
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
                // Avanzamento fase (debug) durante Playing
                if(keyPressed->code == sf::Keyboard::Key::N && gamestate == GameState::Playing && game && !selectingTributes && !ssChoiceActive){
                    game->advancePhase();
                    std::cout << "Fase -> " << phaseToString(game->getTurn().getPhase()) << std::endl;
                    // Reset della selezione tributi fuori dalle fasi Main
                    if(game->getTurn().getPhase() != GamePhase::Main1 && game->getTurn().getPhase() != GamePhase::Main2){
                        selectingTributes = false; selectedTributes.clear(); tributesNeeded = 0;
                    }
                    // Abilita la UI di battaglia solo in Battle
                    attackSelectionActive = (game->getTurn().getPhase() == GamePhase::Battle);
                    if(!attackSelectionActive){ selectedAttackerIndex.reset(); }
                }
                // Entra/Esci dalla finestra di risposta (R) quando è il turno dell'avversario
                if(keyPressed->code == sf::Keyboard::Key::R && gamestate == GameState::Playing && game){
                    int cur = game->getTurn().getCurrentPlayerIndex();
                    // Il lato basso è sempre il giocatore 0. Response window ha senso quando non è il suo turno
                    if(cur != 0){
                        responseWindowActive = !responseWindowActive;
                        battleFeedbackMsg = responseWindowActive ? "Finestra risposta attiva (X per attivare Trap/Rapida)" : "Finestra risposta chiusa";
                        battleFeedbackClock.restart();
                    }
                }
                // Forza fine turno (debug)
                if(keyPressed->code == sf::Keyboard::Key::T && gamestate == GameState::Playing && game && !selectingTributes && !ssChoiceActive){
                    if(game->getTurn().getPhase() != GamePhase::End){
                        game->fastForwardToEndPhase();
                    } else if(!discardController.active() && !game->shouldAutoEndTurn() && !game->hasPendingSpecialSummon()) {
                        // End Phase già processata (scarti avvenuti): termina immediatamente
                        game->endTurn();
                        std::cout << "Nuovo Turno: Player " << (game->getTurn().getCurrentPlayerIndex()+1) << std::endl;
                    }
                }
                // Apertura popup conferma ritorno se stiamo giocando
                if(keyPressed->code == sf::Keyboard::Key::Space && gamestate == GameState::Playing){
                    returnPopupActive = true;
                }
                // DEBUG: O -> aggiungi un mostro al campo avversario
                if(keyPressed->code == sf::Keyboard::Key::O && gamestate == GameState::Playing && game){
                    // Crea una carta mostro base usando la texture flipped (segnaposto)
                    Card dbg("Enemy Dummy", "Test", 1000, 1000,
                        sf::Vector2f(0.f,0.f), sf::Vector2f(0.f,0.f), textureFlipped, Type::Monster, Attribute::None, 4, {});
                    game->debugAddMonsterToOpponent(dbg);
                    syncMonsterZoneToField();
                }
                // Tasto rapido per attacco diretto (disabilitato durante selezione tributi)
                if(keyPressed->code == sf::Keyboard::Key::A && gamestate == GameState::Playing && game){
                    if(game->getTurn().getPhase() == GamePhase::Battle && !selectingTributes && selectedAttackerIndex.has_value()){
                        if(game->canDeclareAttack(selectedAttackerIndex.value(), std::nullopt)){
                            if(game->declareAttack(selectedAttackerIndex.value(), std::nullopt)){
                                battleFeedbackMsg = "Attacco diretto"; battleFeedbackClock.restart();
                            }
                        } else {
                            battleFeedbackMsg = "Attacco non consentito"; battleFeedbackClock.restart();
                        }
                    }
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
                    // Targeting in battaglia: click destro su un mostro avversario per selezionarlo (disabilitato durante selezione tributi)
                    if(but == sf::Mouse::Button::Right && gamestate == GameState::Playing && game && !selectingTributes && game->getTurn().getPhase() == GamePhase::Battle){
                        if(selectedAttackerIndex.has_value()){
                            sf::Vector2i mpos = sf::Mouse::getPosition(window);
                            for(size_t i=0;i<oppFieldCards.size();++i){
                                if(oppFieldCards[i].isClicked(mpos)){
                                    if(game->canDeclareAttack(selectedAttackerIndex.value(), i)){
                                        if(game->declareAttack(selectedAttackerIndex.value(), i)){
                                            battleFeedbackMsg = "Attacco dichiarato"; battleFeedbackClock.restart();
                                        }
                                    } else {
                                        battleFeedbackMsg = "Bersaglio non valido"; battleFeedbackClock.restart();
                                    }
                                    break;
                                }
                            }
                        }
                    }

                if(but == sf::Mouse::Button::Left) {
                    mousePressed = true;
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    initialMousePos = static_cast<sf::Vector2f>(mousePos);

                    // Avvio hold per overlay extra deck e graveyard (disabilitati durante selezione tributi)
                    if(!selectingTributes){
                        extraOverlay.startHold(static_cast<sf::Vector2f>(mousePos), sf::FloatRect(extraDeckSlotPos, slotSize), gamestate);
                        sf::Vector2f gyPos = field.getSlotPosition(Type::Graveyard, P1);
                        bool gyHasCards = false;
                        if(game){ gyHasCards = !game->getGraveyard().empty(); }
                        graveyardOverlay.startHold(static_cast<sf::Vector2f>(mousePos), sf::FloatRect(gyPos, slotSize), gamestate, gyHasCards);
                    }
                    
                    // Gestione click su overlay (disabilitati durante selezione tributi)
                    if(!selectingTributes){
                        extraOverlay.handleMousePress(static_cast<sf::Vector2f>(mousePos), deck.getExtraCards());
                        if(game){
                            graveyardOverlay.handleMousePress(static_cast<sf::Vector2f>(mousePos), game->getGraveyard());
                        }
                    }

                    if(gamestate == GameState::HomeScreen && homeScreen.getGiocaBounds().contains(initialMousePos)) {
                        if(selectedDeckName.empty()) {
                            std::cout << "Tentativo di giocare senza deck selezionato" << std::endl;
                            showNoDeckWarning = true;
                            noDeckWarningClock.restart();
                        } else {
                            std::cout << "Cliccato su Gioca!" << std::endl;
                            
                            // Reset animazione del campo se ha uno stato interno (riavvia transizione FieldLoading->FieldVisible)
                            field.resetAnimation();
                            
                            // Impedisci che si entri direttamente in Playing cancellando eventuale shuffleAnimation terminata precedente
                            deck.clearShuffleAnimationAdvanced();
                            // Ricrea sempre un nuovo Game (anche senza passare di nuovo dalla DeckSelection)
                            Player p1("Player1", deck);
                            // Usa un mazzo non vuoto anche per Player 2 per evitare deck-out immediato
                            Player p2("Player2", deck);
                            game = std::make_unique<Game>(p1, p2);
                            // Registra effetti carta (assicurati anche nel flusso "Gioca!" da Home)
                            game->registerEffectForCardName("Pietra Bianca della Leggenda", std::make_unique<WhiteStoneLegendEffect>());
                            game->registerEffectForCardName("Pietra Bianca degli Antichi", std::make_unique<WhiteStoneAncientsEffect>());
                            game->attachExternalDeck(&deck);
                            game->attachDrawController(&drawController);
                            handPtr = &game->current().getHand();
                            if(handPtr) handPtr->clear();
                            // Le sottoscrizioni agli eventi verranno impostate dopo l'avvio del gioco
                            gamestate = GameState::FieldLoading; // stato intermedio con animazione
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
                if(handPtr && !selectingTributes){
                for (size_t i = 0; i < handPtr->size(); ++i) {
                    if ((*handPtr)[i].isClicked(mousePos)) {
                        isPotentialDrag = true;
                        potentialDragCardIndex = i;
                        sf::Vector2f cardPos = (*handPtr)[i].getPosition();
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
        }
                
                // Se non è stata cliccata una carta in mano, controlla le carte sul campo
                if (!cardClicked) {
                    for (size_t i = 0; i < fieldCards.size(); ++i) {
                        if (fieldCards[i].isClicked(mousePos)) {
                            cardClicked = true;
                            selectedCardIndex = i; 
                            selectedCardIsOnField = true; // Carta sul campo
                            scrollOffset = 0.f;
                            if(selectingTributes){
                                // Toggle della selezione tributo
                                auto it = std::find(selectedTributes.begin(), selectedTributes.end(), i);
                                if(it == selectedTributes.end()){
                                    if(static_cast<int>(selectedTributes.size()) < tributesNeeded){
                                        selectedTributes.push_back(i);
                                        std::cout << "[Tribute] Aggiunto mostro indice zona " << i << std::endl;
                                    }
                                } else {
                                    selectedTributes.erase(it);
                                    std::cout << "[Tribute] Rimosso mostro indice zona " << i << std::endl;
                                }
                                // Se è stato raggiunto il numero richiesto prova a completare
                                if(game && tributesNeeded>0 && static_cast<int>(selectedTributes.size()) == tributesNeeded){
                                    if(game->completePendingNormalSummon(selectedTributes)){
                                        selectingTributes = false;
                                        selectedTributes.clear();
                                        tributesNeeded = 0;
                                        // Aggiorna il render dopo il completamento dei tributi
                                        syncMonsterZoneToField();
                                        if(handPtr){
                                            updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                                        }
                                        std::cout << "[Tribute] Summon completata" << std::endl;
                                    } else {
                                        std::cout << "[Tribute] Completamento summon fallito" << std::endl;
                                    }
                                }
                            }
                            // Battaglia: seleziona l'attaccante con il click (disabilitato durante selezione tributi)
                            if(game && !selectingTributes && game->getTurn().getPhase() == GamePhase::Battle){
                                // Evita di selezionare mostri che hanno già attaccato
                                if(!game->hasMonsterAlreadyAttacked(i)){
                                    attackSelectionActive = true;
                                    selectedAttackerIndex = i;
                                    battleFeedbackMsg = "Attaccante selezionato";
                                    battleFeedbackClock.restart();
                                } else {
                                    battleFeedbackMsg = "Questo mostro ha gia' attaccato";
                                    battleFeedbackClock.restart();
                                }
                            }
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
                    graveyardOverlay.handleMouseRelease();

                    //funzione per far tornare la carta in mano in caso di posizione errata
                    auto cardBackToHand = [&](){
                        Card tempCard = (*handPtr)[draggingCardIndex.value()];
                        handPtr->erase(handPtr->begin() + draggingCardIndex.value());
                        sf::Vector2f originalPos = setHandPos(*handPtr, tempCard, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                        handPtr->insert(handPtr->begin() + draggingCardIndex.value(), tempCard);
                        (*handPtr)[draggingCardIndex.value()].setPosition(originalPos);
                        updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                    };

                    if (isDragging && draggingCardIndex.has_value()) {
                        
                        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                        
                        if (handPtr && findSlotPosition(mousePos, (*handPtr)[draggingCardIndex.value()], field, slotSize).has_value()) {
                            // Mostra scelta Evocazione/Posizionamento oppure Attiva/Setta
                            if(game && !selectingTributes){
                                pendingHandIndexForPlay = draggingCardIndex.value();
                                Type t = (*handPtr)[draggingCardIndex.value()].getType();
                                if(t == Type::Monster){
                                    pendingIsSpellTrap = false;
                                    chooseSummonOrSet = true;
                                    battleFeedbackMsg = "Premi E per Evocare, P per Posizionare";
                                } else if(t == Type::SpellTrap || t == Type::FieldSpell){
                                    pendingIsSpellTrap = true;
                                    chooseActivateOrSetST = true;
                                    battleFeedbackMsg = "Magia/Trappola: A per Attivare, S per Settare";
                                } else {
                                    // fallback sui mostri
                                    pendingIsSpellTrap = false;
                                    chooseSummonOrSet = true;
                                    battleFeedbackMsg = "Premi E per Evocare, P per Posizionare";
                                }
                                battleFeedbackClock.restart();
                            } else if(selectingTributes){
                                std::cout << "[Tribute] Seleziona i tributi richiesti" << std::endl;
                            } else {
                                if(handPtr){ cardBackToHand(); }
                            }
                        } else {
                            // Posizione non valida: calcola la posizione originale nella mano e riposiziona
                            if(handPtr){
                                cardBackToHand();
                            }
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
                        graveyardOverlay.handleEscape();
                        // Reset battle selection
                        selectedAttackerIndex.reset();
                        attackSelectionActive = false;
                        // Se stiamo selezionando tributi: annulla la selezione e la summon pendente
                        if(selectingTributes){
                            if(game) game->cancelPendingNormalSummon();
                            selectingTributes = false; selectedTributes.clear(); tributesNeeded = 0;
                            battleFeedbackMsg.clear();
                            // Resync render
                            syncMonsterZoneToField();
                        }
                        if(attackSelectionActive){
                            attackSelectionActive = false;
                            selectedAttackerIndex.reset();
                            battleFeedbackMsg.clear();
                        }
                        if(chooseSummonOrSet || chooseActivateOrSetST){
                            // Annulla scelta e ripristina carta nella mano
                            chooseSummonOrSet = false;
                            chooseActivateOrSetST = false;
                            if(pendingHandIndexForPlay.has_value()){
                                restoreCardAtHandIndex(pendingHandIndexForPlay.value());
                            }
                            pendingHandIndexForPlay.reset();
                            pendingIsSpellTrap = false;
                        }
                        // Chiudi eventuale response window
                        responseWindowActive = false;
                    }
                }
                // Scelta Evocazione/Posizionamento
                if(chooseSummonOrSet && game && pendingHandIndexForPlay.has_value()){
                    if(keyPressed->code == sf::Keyboard::Key::E){
                        if(game->tryNormalSummon(pendingHandIndexForPlay.value())){
                            if(handPtr){ updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE); }
                        } else {
                            battleFeedbackMsg = "Evocazione non riuscita";
                            battleFeedbackClock.restart();
                            // Ripristina carta in mano
                            restoreCardAtHandIndex(pendingHandIndexForPlay.value());
                        }
                        chooseSummonOrSet = false;
                        pendingHandIndexForPlay.reset();
                    } else if(keyPressed->code == sf::Keyboard::Key::P){
                        if(game->tryNormalSet(pendingHandIndexForPlay.value())){
                            if(handPtr){ updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE); }
                        } else {
                            battleFeedbackMsg = "Posizionamento non riuscito";
                            battleFeedbackClock.restart();
                            // Ripristina carta in mano
                            restoreCardAtHandIndex(pendingHandIndexForPlay.value());
                        }
                        chooseSummonOrSet = false;
                        pendingHandIndexForPlay.reset();
                    }
                }

                // Nuovo toggle posizione: un solo tasto (C)
                if(gamestate == GameState::Playing && game && !selectingTributes && !chooseSummonOrSet){
                    if(keyPressed->code == sf::Keyboard::Key::C){
                        if(selectedCardIsOnField && selectedCardIndex.has_value()){
                            if(game->togglePosition(selectedCardIndex.value())){
                                battleFeedbackMsg = "Posizione cambiata";
                                battleFeedbackClock.restart();
                            }
                        }
                    } else if(keyPressed->code == sf::Keyboard::Key::X){
                        // Attiva una M/T settata sotto al mouse
                        sf::Vector2i mpos = sf::Mouse::getPosition(window);
                        bool triggered = false;
                        int cur = game->getTurn().getCurrentPlayerIndex();
                        if(cur == 0){
                            // Tuo turno: attivi solo le tue M/T (P1)
                            for(size_t i=0;i<p1STCards.size();++i){
                                if(p1STCards[i].getGlobalBounds().contains(sf::Vector2f((float)mpos.x, (float)mpos.y))){
                                    if(game->activateSetSpellTrap(i)){
                                        battleFeedbackMsg = "Carta attivata"; battleFeedbackClock.restart();
                                        syncSpellTrapZones();
                                    } else {
                                        battleFeedbackMsg = "Impossibile attivarla ora"; battleFeedbackClock.restart();
                                    }
                                    triggered = true; break;
                                }
                            }
                        } else {
                            // Turno avversario: se la response window è attiva, consenti l'attivazione delle tue M/T (P1)
                            if(responseWindowActive){
                                for(size_t i=0;i<p1STCards.size();++i){
                                    if(p1STCards[i].getGlobalBounds().contains(sf::Vector2f((float)mpos.x, (float)mpos.y))){
                                        if(game->activateSetSpellTrapFor(0, i)){
                                            battleFeedbackMsg = "Carta attivata"; battleFeedbackClock.restart();
                                            syncSpellTrapZones();
                                        } else {
                                            battleFeedbackMsg = "Impossibile attivarla ora"; battleFeedbackClock.restart();
                                        }
                                        triggered = true; break;
                                    }
                                }
                            }
                        }
                    }
                }
            }    
             
            // Scroll mouse
            if (const auto* mouseScroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                // Permetti lo scroll sia se una carta è selezionata, sia se si sta mostrando la carta pescata
                if (selectedCardIndex.has_value() || showDrawnCardDetails || extraOverlay.detailsActive() || graveyardOverlay.detailsActive()) {
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
        // Pausa logica/animazioni se popup o game over attivi (deltaTime annullato)
        if(returnPopupActive || gameOverActive) deltaTime = 0.f;
    
        // Se è stato selezionato un deck, caricalo e re-inizializza lo stato della mano/pescate
        if(deckSelected){
            deck = resourceManager.getDeckByName(selectedDeckName);

            // Reimposta lo stato di gioco relativo alla mano e alle animazioni di pescata
            if(handPtr) handPtr->clear();
            fieldCards.clear();
            drawController.reset();
            selectedCardIndex.reset();
            selectedCardIsOnField = false;

            // Assicura che il nome sia aggiornato (già fatto nell'handler ma nel dubbio)
            homeScreen.setDeckName(selectedDeckName);

            // Crea due player: P1 = deck selezionato, P2 = dummy (per test locali)
            Player p1("Player1", deck);
            // Usa lo stesso deck anche per Player 2 per test locali (evita deck-out)
            Player p2("Player2", deck);
            game = std::make_unique<Game>(p1, p2);
            // Registra effetti base (per tipo). Iniziamo con "Pietra Bianca della Leggenda" (search BEWD on GY)
            game->registerEffectForCardName("Pietra Bianca della Leggenda", std::make_unique<WhiteStoneLegendEffect>());
            game->registerEffectForCardName("Pietra Bianca degli Antichi", std::make_unique<WhiteStoneAncientsEffect>());
            game->attachExternalDeck(&deck);
            game->attachDrawController(&drawController);
            handPtr = &game->current().getHand(); // getHand() deve restituire non-const (assicurato)
            handPtr->clear();
            deckSelected = false; // Evita di rieseguire ogni frame    
        }
        
    // Gestione dell'animazione FieldLoading
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

    // Controlla se il mouse si è mosso abbastanza e nella direzione corretta per attivare il dragging (bloccato durante la selezione tributi)
        if (isPotentialDrag && potentialDragCardIndex.has_value() && !selectingTributes) {
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

                    // Avvia la shuffle animation avanzata una sola volta
                    if (!shuffleStarted) {
                        deck.startShuffleAnimationAdvanced(deckSlotPos, deckCardSize);
                        shuffleStarted = true;
                    }
                    // Aggiorna la shuffle animation
                    deck.updateShuffleAnimationAdvanced(deltaTime);
                    // Quando l'animazione è finita, mischia effettivamente il deck e passa allo stato Playing
                    if (deck.isShuffleAnimationAdvancedFinished()) {
                        deck.shuffle();
                        deck.resetDeckCardPositions(deckSlotPos, deckCardSize, slotSize, textureFlipped);
                        std::cout << "Passaggio allo stato Playing..." << std::endl;
                        gamestate = GameState::Playing;
                        
                        if(game && !game->isStarted()){
                            game->start();
                            // Prima sync completa delle zone
                            syncMonsterZoneToField();
                            syncSpellTrapZones();
                            drawStartSubscriptionId = game->events().subscribe(GameEventType::DrawStart, [](){
                                std::cout << "[Event] DrawStart" << std::endl;
                            });
                            drawEndSubscriptionId = game->events().subscribe(GameEventType::DrawEnd, [](){
                                std::cout << "[Event] DrawEnd" << std::endl;
                            });
                            game->events().subscribe(GameEventType::NormalSummonTributeRequired, [&](){
                                selectingTributes = true;
                                selectedTributes.clear();
                                tributesNeeded = game->getPendingTributesNeeded();
                                std::cout << "[Event] NormalSummonTributeRequired" << std::endl;
                                // Ferma eventuali drag in corso per evitare di muovere carte in mano durante la selezione tributi
                                isDragging = false; isPotentialDrag = false; draggingCardIndex.reset(); potentialDragCardIndex.reset();
                            });
                            game->events().subscribe(GameEventType::MonstersTributed, [&](){
                                std::cout << "[Event] MonstersTributed" << std::endl;
                                syncMonsterZoneToField();
                            });
                            game->events().subscribe(GameEventType::NormalSummon, [&](){
                                std::cout << "[Event] NormalSummon" << std::endl;
                                syncMonsterZoneToField();
                                syncSpellTrapZones();
                            });
                            game->events().subscribe(GameEventType::NormalSet, [&](){
                                std::cout << "[Event] NormalSet" << std::endl;
                                syncMonsterZoneToField();
                                syncSpellTrapZones();
                            });
                            // Cambio turno: riallinea le zone così il rendering corrisponde al giocatore corrente
                            game->events().subscribe(GameEventType::TurnStart, [&](){
                                std::cout << "[Event] TurnStart" << std::endl;
                                syncMonsterZoneToField();
                                syncSpellTrapZones();
                            });
                            game->events().subscribe(GameEventType::TurnEnd, [&](){
                                std::cout << "[Event] TurnEnd" << std::endl;
                                // Facoltativo, ma mantiene la UI coerente anche prima del nuovo TurnStart
                                syncMonsterZoneToField();
                                syncSpellTrapZones();
                            });
                            // Eventi di battaglia
                            attackDeclaredSubId = game->events().subscribe(GameEventType::AttackDeclared, [&](){
                                std::cout << "[Event] AttackDeclared" << std::endl;
                            });
                            attackResolvedSubId = game->events().subscribe(GameEventType::AttackResolved, [&](){
                                std::cout << "[Event] AttackResolved" << std::endl;
                                syncMonsterZoneToField();
                                selectedAttackerIndex.reset();
                            });
                            monsterDestroyedSubId = game->events().subscribe(GameEventType::MonsterDestroyed, [&](){
                                std::cout << "[Event] MonsterDestroyed" << std::endl;
                            });
                            lifeChangedSubId = game->events().subscribe(GameEventType::LifePointsChanged, [&](){
                                std::cout << "[Event] LifePointsChanged" << std::endl;
                                battleFeedbackMsg = "LP cambiati";
                                battleFeedbackClock.restart();
                            });
                            // Aggiorna il render su spostamenti generici (es. cambio posizione)
                            game->events().subscribe(GameEventType::CardMoved, [&](){
                                std::cout << "[Event] CardMoved" << std::endl;
                                // Riallinea le zone mostri per eventuali spostamenti sul campo
                                syncMonsterZoneToField();
                                syncSpellTrapZones();
                                // Aggiorna layout della mano se è cambiata (es. aggiunta carta da un effetto)
                                if(handPtr){
                                    // Garantisci che le carte in mano mostrino sempre il fronte
                                    for(auto &c : *handPtr){
                                        if(!c.originalTexturePath.empty()){
                                            sf::Texture &front = textureManager.getTexture(c.originalTexturePath);
                                            c.setTexture(front);
                                            sf::Vector2u ts = front.getSize();
                                            c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i(ts.x, ts.y)));
                                        }
                                    }
                                    // Poi riallinea posizioni/dimensioni della mano
                                    updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                                }
                            });
                            game->events().subscribe(GameEventType::MonsterFlipped, [&](){
                                std::cout << "[Event] MonsterFlipped" << std::endl;
                                battleFeedbackMsg = "Flip!";
                                battleFeedbackClock.restart();
                                syncMonsterZoneToField();
                                syncSpellTrapZones();
                            });
                            game->events().subscribe(GameEventType::SpellSet, [&](){
                                battleFeedbackMsg = "Magia settata"; battleFeedbackClock.restart();
                                syncSpellTrapZones();
                            });
                            game->events().subscribe(GameEventType::TrapSet, [&](){
                                battleFeedbackMsg = "Trappola settata"; battleFeedbackClock.restart();
                                syncSpellTrapZones();
                            });
                            game->events().subscribe(GameEventType::SpellActivated, [&](){
                                battleFeedbackMsg = "Magia attivata"; battleFeedbackClock.restart();
                                syncSpellTrapZones();
                            });
                            game->events().subscribe(GameEventType::TrapActivated, [&](){
                                battleFeedbackMsg = "Trappola attivata"; battleFeedbackClock.restart();
                                syncSpellTrapZones();
                            });
                            // Richiesta di scelta per Evocazione Speciale (Attacco o Difesa, mai coperto)
                            game->events().subscribe(GameEventType::SpecialSummonChoiceRequested, [&](){
                                std::cout << "[Event] SpecialSummonChoiceRequested" << std::endl;
                                ssChoiceActive = true;
                                ssChoiceOwner = game->getTurn().getCurrentPlayerIndex();
                                // Ferma input conflittuali
                                isDragging = false; isPotentialDrag = false; draggingCardIndex.reset(); potentialDragCardIndex.reset();
                            });
                            directAttackSubId = game->events().subscribe(GameEventType::DirectAttack, [&](){
                                std::cout << "[Event] DirectAttack" << std::endl;
                            });
                            // Vittoria/Sconfitta -> attiva l'overlay
                            game->events().subscribe(GameEventType::Win, [&](){
                                if(!gameOverActive){
                                    gameOverActive = true;
                                }
                            });
                            game->events().subscribe(GameEventType::Lose, [&](){
                                if(!gameOverActive){
                                    gameOverActive = true;
                                }
                            });
                            // Listener per lo scarto automatico quando si entra in End Phase
                            game->setHandLimit(HAND_MAXSIZE);
                            game->setDiscardCallback([&](std::vector<Card>&& excess){
                                if(!handPtr) return;
                                if(excess.empty()) return; // nessun scarto
                                sf::Vector2f gyPos = field.getSlotPosition(Type::Graveyard, P1);
                                sf::Vector2f gyCenter(gyPos.x + slotSize.x*0.5f, gyPos.y + slotSize.y*0.5f);
                                discardController.start(std::move(excess), gyCenter);
                                updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                                std::cout << "[AutoDiscard] Scarto animato avviato." << std::endl;
                            });
                        }
                    }
                }
            }
        }

    // Avvia la prima animazione di pescata SOLO dopo il passaggio allo stato Playing
    // (La pescata iniziale "legacy" è sostituita dal DrawController nella fase Draw)

        Card tmpcard = Card("", "", 0, 0, sf::Vector2f(0.f, 0.f), sf::Vector2f(0.f, 0.f), textureFlipped, Type::Monster, Attribute::None, 0, {});

    if(gamestate == GameState::Playing && !returnPopupActive){
            if(game && handPtr){
                drawController.update(deltaTime, mousePressed, deck, *handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE, deckSlotPos, textureFlipped, textureManager, &game->events());
                if(drawController.isShowingDetails()){
                    showDrawnCardDetails = true; tmpcard = drawController.detailCard();
                } else if(showDrawnCardDetails && drawController.idle()){
                    showDrawnCardDetails = false;
                }
                // Aggiorna animazioni di scarto
                discardController.update(deltaTime, *game);
                if(game->getTurn().getPhase() == GamePhase::End){
                    if(!discardController.active()){
                        if(game->shouldAutoEndTurn()){
                            game->onDiscardAnimationFinished();
                            std::cout << "[AutoTurn] Fine turno automatica (nessuno scarto)." << std::endl;
                        }
                    } else if(discardController.active()) {
                        // appena terminata animazione scarto
                        updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                        game->onDiscardAnimationFinished();
                        std::cout << "[AutoTurn] Fine turno dopo animazione scarto." << std::endl;
                    }
                }
                // Gestione input per overlay scelta Evocazione Speciale (A=Attacco, D=Difesa)
                if(ssChoiceActive){
                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)){
                        if(game->resolvePendingSpecialSummon(/*defense*/false)){
                            ssChoiceActive = false; ssChoiceOwner = -1;
                        }
                    } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)){
                        if(game->resolvePendingSpecialSummon(/*defense*/true)){
                            ssChoiceActive = false; ssChoiceOwner = -1;
                        }
                    }
                }
                // Scelta post-drop per Spell/Trap: A=Attiva, S=Setta
                if(pendingHandIndexForPlay.has_value() && pendingIsSpellTrap && chooseActivateOrSetST){
                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)){
                        if(game->activateSpellFromHand(pendingHandIndexForPlay.value())){
                            updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                        } else {
                            restoreCardAtHandIndex(pendingHandIndexForPlay.value());
                            battleFeedbackMsg = "Attivazione non riuscita"; battleFeedbackClock.restart();
                        }
                        chooseActivateOrSetST = false; pendingHandIndexForPlay.reset(); pendingIsSpellTrap = false;
                    } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)){
                        size_t idx = pendingHandIndexForPlay.value();
                        bool ok = false;
                        if((*handPtr)[idx].getType() == Type::FieldSpell){ ok = game->setFieldSpell(idx); }
                        else { ok = game->setSpellOrTrap(idx); }
                        if(ok){
                            updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                        } else {
                            restoreCardAtHandIndex(idx);
                            battleFeedbackMsg = "Set non riuscito"; battleFeedbackClock.restart();
                        }
                        chooseActivateOrSetST = false; pendingHandIndexForPlay.reset(); pendingIsSpellTrap = false;
                    }
                }
            }
        }
    

    // Gestione dell'hold dell'Extra Deck (attiva la preview dopo una soglia se non si stanno trascinando carte)
        if(!returnPopupActive){
            extraOverlay.update(deltaTime, isDragging, gamestate);
            graveyardOverlay.update(deltaTime, isDragging, gamestate);
        }

    // Gestione del sollevamento delle carte in mano
        float targetCardOffset = 0.f;
        float offsetSpeed = 200.f; 
        if(!returnPopupActive){
            if(handPtr){
            for(size_t i = 0; i<handPtr->size(); ++i){
                float currentOffset = (*handPtr)[i].getOffset();
                if(selectedCardIndex.has_value() && !selectedCardIsOnField && selectedCardIndex.value() == i) {
                    targetCardOffset = CARD_MAXOFFSET;
                } else {
                    targetCardOffset = 0.f; 
                }
                if(currentOffset < targetCardOffset) {
                    currentOffset = std::min(currentOffset + offsetSpeed * deltaTime, targetCardOffset); 
                    (*handPtr)[i].setOffset(currentOffset);
                } else {
                    currentOffset = std::max(currentOffset - offsetSpeed * deltaTime, targetCardOffset);
                    (*handPtr)[i].setOffset(currentOffset);
                }    
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
        if (isDragging && draggingCardIndex.has_value() && handPtr && draggingCardIndex.value() < handPtr->size()) {
            sf::Vector2f newCardPos = static_cast<sf::Vector2f>(mousePos) - dragOffset;
            (*handPtr)[draggingCardIndex.value()].setPosition(newCardPos);
        }

    // Se siamo nello stato StartScreen, disegna il testo sopra lo sfondo con animazione di dissolvenza
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
                bool enableHover = !extraOverlay.isOverlayVisible() && !returnPopupActive && !chooseSummonOrSet && !chooseActivateOrSetST;
                field.draw(window, mousePos, gamestate, enableHover);
            }

            // Disegna il Deck principale (o l'animazione di mescolamento) e l'Extra Deck sopra il relativo slot
            if(field.isAnimationFinished()) {
                if (deck.getSize() > 0) {
                    if (!deck.isShuffleAnimationAdvancedFinished()) {
                        deck.drawShuffleAnimationAdvanced(window);
                    } else {
                        deck.draw(window, mousePos, detailFont, deckSlotPos, slotSize, gamestate);
                    }
                }
                // Extra Deck: disegna sempre lo stack se contiene carte
                if (deck.getExtraSize() > 0) {
                    deck.drawExtra(window, detailFont, extraDeckSlotPos, slotSize, gamestate);
                }
            }

            if (showDrawnCardDetails) {
                sf::Vector2f panelPos{400.f, 150.f};
                sf::Vector2f panelSize{300.f, 200.f};
                showCardDetails(window, tmpcard, detailFont, panelPos, panelSize, scrollOffset);
            }

            // Disegna le carte sul campo (P1)
            for (size_t i=0;i<fieldCards.size();++i) {
                sf::Color color = sf::Color::White;
                if(selectedAttackerIndex.has_value() && selectedAttackerIndex.value()==i && attackSelectionActive){
                    color = sf::Color(255,240,160); // evidenziazione con tinta chiara
                }
                bool isDef=false, isFD=false;
                if(game){
                    // Il lato basso corrisponde sempre al Giocatore 0
                    isDef = game->isDefenseAt(0, i);
                    isFD = game->isFaceDownAt(0, i);
                }
                Card drawCard = fieldCards[i];
                // Le carte coperte devono usare la texture del retro
                if(isFD){ 
                    drawCard.setTexture(textureFlipped);
                    sf::Vector2u texSize = textureFlipped.getSize();
                    drawCard.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texSize.x, texSize.y)));

                }
                // Ruota orizzontalmente se in DIFESA o coperta
                if(isDef || isFD){
                    // Usa una posizione centrata nello slot e adatta la dimensione per il layout orizzontale
                    sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, P1, static_cast<int>(i));
                    sf::Vector2f slotCenter(slotPos.x + slotSize.x/2.f, slotPos.y + slotSize.y/2.f);
                    // Dimensione target prima della rotazione:
                    //  - larghezza (sottile) ~ 60% dell'altezza dello slot
                    //  - altezza (lunga) ~ 95% della larghezza dello slot (così dopo 90° la base orizzontale è lunga)
                    sf::Vector2f rotatedSize(slotSize.y * 0.60f, slotSize.x * 0.95f);
                    drawCard.setSize(rotatedSize);
                    drawCard.centerOrigin();
                    drawCard.setRotation(90.f);
                    drawCard.setPosition(slotCenter);
                }
                if(!isDef && !isFD){
                    // Carta in piedi: nessuna rotazione; dimensione/posizione sono già impostate dalla sync
                    drawCard.resetOrigin();
                    drawCard.setRotation(0.f);
                }
                if(isDef){
                    auto mix = [](sf::Color a, sf::Color b){
                        return sf::Color(
                            static_cast<uint8_t>(std::min(255, (int(a.r)+int(b.r))/2)),
                            static_cast<uint8_t>(std::min(255, (int(a.g)+int(b.g))/2)),
                            static_cast<uint8_t>(std::min(255, (int(a.b)+int(b.b))/2)),
                            a.a
                        );
                    };
                    color = mix(color, sf::Color(150,200,255,color.a));
                }
                drawCard.draw(window, color);
                // Indicatore "ha attaccato": piccolo pallino rosso in alto a sinistra della carta
                // Disegna il pallino d'attacco solo sul lato del giocatore di turno (in basso se cur==0, in alto se cur==1)
                if(game && game->getTurn().getPhase() == GamePhase::Battle){
                    int cur = game->getTurn().getCurrentPlayerIndex();
                    if(cur == 0 && game->hasMonsterAlreadyAttacked(i)){
                    sf::FloatRect gb = fieldCards[i].getGlobalBounds();
                    float r = 9.f;
                    sf::CircleShape dot(r);
                    dot.setFillColor(sf::Color(220,60,60,230));
                    dot.setOutlineColor(sf::Color(0,0,0,200));
                    dot.setOutlineThickness(2.f);
                    dot.setPosition(sf::Vector2f(gb.position.x + 6.f, gb.position.y + 6.f));
                    window.draw(dot);
                    }
                }
            }
            // Disegna anche le Magie/Trappole e Campo (P1)
            if(game){
                for(size_t i=0;i<p1STCards.size();++i){
                    Card c = p1STCards[i];
                    bool isFD = game->isSpellTrapFaceDownAt(0, i);
                    if(isFD){
                        c.setTexture(textureFlipped);
                        sf::Vector2u ts = textureFlipped.getSize();
                        c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i((int)ts.x,(int)ts.y)));
                    }
                    c.draw(window);
                }
                if(p1FieldSpellCard.has_value()){
                    p1FieldSpellCard.value().draw(window);
                }
            }
            // Disegna le carte dell'avversario (P2)
            for (size_t i=0;i<oppFieldCards.size();++i) {
                Card drawCard = oppFieldCards[i];
                sf::Color color = sf::Color::White;
                bool isDef=false, isFD=false;
                if(game){
                    // Il lato alto corrisponde sempre al Giocatore 1
                    isDef = game->isDefenseAt(1, i);
                    isFD = game->isFaceDownAt(1, i);
                }
                // Anche le carte coperte dell'avversario usano la texture del retro
                if(isFD){ drawCard.setTexture(textureNonFlipped); }
                // Ruota orizzontalmente se in DIFESA o coperta
                if(isDef || isFD){
                    sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, P2, static_cast<int>(i));
                    sf::Vector2f slotCenter(slotPos.x + slotSize.x/2.f, slotPos.y + slotSize.y/2.f);
                    sf::Vector2f rotatedSize(slotSize.y * 0.60f, slotSize.x * 0.95f);
                    drawCard.setSize(rotatedSize);
                    drawCard.centerOrigin();
                    drawCard.setRotation(90.f);
                    drawCard.setPosition(slotCenter);
                }
                if(!isDef && !isFD){
                    drawCard.resetOrigin();
                    drawCard.setRotation(0.f);
                }
                if(isDef){
                    auto mix = [](sf::Color a, sf::Color b){
                        return sf::Color(
                            static_cast<uint8_t>(std::min(255, (int(a.r)+int(b.r))/2)),
                            static_cast<uint8_t>(std::min(255, (int(a.g)+int(b.g))/2)),
                            static_cast<uint8_t>(std::min(255, (int(a.b)+int(b.b))/2)),
                            a.a
                        );
                    };
                    color = mix(color, sf::Color(150,200,255,color.a));
                }
                // Pallino d'attacco per il giocatore di turno quando il corrente è il Giocatore 1 (lato alto)
                if(game && game->getTurn().getPhase() == GamePhase::Battle){
                    int cur = game->getTurn().getCurrentPlayerIndex();
                    if(cur == 1 && game->hasMonsterAlreadyAttacked(i)){
                        sf::FloatRect gb = oppFieldCards[i].getGlobalBounds();
                        float r = 9.f;
                        sf::CircleShape dot(r);
                        dot.setFillColor(sf::Color(220,60,60,230));
                        dot.setOutlineColor(sf::Color(0,0,0,200));
                        dot.setOutlineThickness(2.f);
                        dot.setPosition(sf::Vector2f(gb.position.x + 6.f, gb.position.y + 6.f));
                        window.draw(dot);
                    }
                }
                drawCard.draw(window, color);
            }
            // Magie/Trappole e Campo (P2)
            if(game){
                for(size_t i=0;i<p2STCards.size();++i){
                    Card c = p2STCards[i];
                    bool isFD = game->isSpellTrapFaceDownAt(1, i);
                    if(isFD){
                        c.setTexture(textureNonFlipped);
                        sf::Vector2u ts = textureNonFlipped.getSize();
                        c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i((int)ts.x,(int)ts.y)));
                    }
                    c.draw(window);
                }
                if(p2FieldSpellCard.has_value()){
                    p2FieldSpellCard.value().draw(window);
                }
            }

            // Disegna la pila Cimitero (semplice: ultima carta)
            if(game && !game->getGraveyard().empty()){
                const Card &top = game->getGraveyard().back();
                Card copy = top; // disegna una copia centrata nello slot e scalata (senza leggere la size originale)
                sf::Vector2f gyPos = field.getSlotPosition(Type::Graveyard, P1);
                float scale = 0.75f;
                sf::Vector2f gySize(slotSize.x*scale, slotSize.y*scale);
                copy.setSize(gySize);
                copy.setPosition(gyPos + sf::Vector2f((slotSize.x-gySize.x)/2.f, (slotSize.y-gySize.y)/2.f));
                copy.draw(window);
            }

            // Disegna la pila Cimitero dell'avversario (semplice: ultima carta)
            if(game && !game->getOpponentGraveyard().empty()){
                const Card &top = game->getOpponentGraveyard().back();
                Card copy = top; // disegna una copia centrata nello slot e scalata (senza leggere la size originale)
                sf::Vector2f gyPos = field.getSlotPosition(Type::Graveyard, P2);
                float scale = 0.75f;
                sf::Vector2f gySize(slotSize.x*scale, slotSize.y*scale);
                copy.setSize(gySize);
                copy.setPosition(gyPos + sf::Vector2f((slotSize.x-gySize.x)/2.f, (slotSize.y-gySize.y)/2.f));
                copy.draw(window);
            }

            // Disegna le animazioni di scarto in volo (controller)
            discardController.draw(window);
            
            //Disegna le animazioni delle carte
            if(deck.isAnimationFinished()) drawController.draw(window);

            // Disegna le carte in mano
            if(handPtr){
            for (size_t i = 0; i < handPtr->size(); ++i) {
                sf::Vector2f originalPos = (*handPtr)[i].getPosition();
                float offset = ssChoiceActive ? 0.f : (*handPtr)[i].getOffset();
                (*handPtr)[i].setPosition(originalPos - sf::Vector2f(0.f, offset));
                (*handPtr)[i].draw(window);
                (*handPtr)[i].setPosition(originalPos);
            }
            }

            // Se c'è una carta selezionata, mostra i dettagli - Non influisce sullo stato del gioco, ma solo sulla visualizzazione (rendering)
            if (selectedCardIndex.has_value()) {
                const Card* selectedCard = nullptr;
                
                // Determina quale carta è selezionata (in mano o sul campo)
                if (selectedCardIsOnField && selectedCardIndex.value() < fieldCards.size()) {
                    selectedCard = &fieldCards[selectedCardIndex.value()];
                } else if (!selectedCardIsOnField && handPtr && selectedCardIndex.value() < handPtr->size()) {
                    selectedCard = &(*handPtr)[selectedCardIndex.value()];
                }
                
                // Mostra i dettagli solo se abbiamo una carta valida
                if (selectedCard != nullptr) {
                    sf::Vector2f panelPos{400.f, 150.f};
                    sf::Vector2f panelSize{300.f, 200.f};
                    showCardDetails(window, *selectedCard, detailFont, panelPos, panelSize, scrollOffset);
                }
            }

            // Overlay di anteprima dell'Extra Deck (mostra tutte le carte) - sopra tutto il resto ma sotto eventuali pannelli dettagli
            // Overlay Extra Deck
            extraOverlay.draw(window, detailFont, deck.getExtraCards(), scrollOffset);
            if(game){
                graveyardOverlay.draw(window, detailFont, game->getGraveyard(), scrollOffset);
            }
            if(extraOverlay.detailsActive() || graveyardOverlay.detailsActive()) {
                const Card* c = extraOverlay.detailsActive() ? extraOverlay.getSelectedCard() : graveyardOverlay.getSelectedCard();
                if(c){
                    sf::Vector2f panelPos{ windowSize.x * 0.07f, windowSize.y * 0.12f };
                    sf::Vector2f panelSize{ 340.f, 260.f };
                    showCardDetails(window, *c, detailFont, panelPos, panelSize, scrollOffset);
                }
            }
            // HUD turno/fase (semplice)
            if(game){
                size_t gyCount = game->getGraveyard().size();
                size_t handCount = handPtr ? handPtr->size() : 0;
                int over = static_cast<int>(handCount) - HAND_MAXSIZE;
                std::string hud = "Turno " + std::to_string(game->getTurn().getTurnCount()) +
                    " - Player " + std::to_string(game->getTurn().getCurrentPlayerIndex()+1) +
                    " - Phase: " + phaseToString(game->getTurn().getPhase()) +
                    " | Hand: " + std::to_string(handCount) + (over>0?" (+"+std::to_string(over)+")":"") +
                    " | GY: " + std::to_string(gyCount) +
                    "  (N = next phase, T = end turn)";
                sf::Text hudText(detailFont, hud, 22);
                hudText.setFillColor(sf::Color(250,250,250,220));
                hudText.setOutlineColor(sf::Color(0,0,0,180));
                hudText.setOutlineThickness(2.f);
                hudText.setPosition(sf::Vector2f(15.f, 10.f));
                window.draw(hudText);
                // LP HUD in alto a destra
                int lpCur = game->current().getLifePoints();
                int lpOpp = game->opponent().getLifePoints();
                std::string lpHud = std::string("LP ") + std::to_string(lpCur) + " | Opp " + std::to_string(lpOpp);
                sf::Text lpText(detailFont, lpHud, 22);
                lpText.setFillColor(sf::Color(255,230,140,230));
                lpText.setOutlineColor(sf::Color(0,0,0,180));
                lpText.setOutlineThickness(2.f);
                sf::FloatRect lpBounds = lpText.getLocalBounds();
                lpText.setPosition(sf::Vector2f(windowSize.x - lpBounds.size.x - 25.f, 10.f));
                window.draw(lpText);
                if(selectingTributes){
                    sf::Text trib(detailFont, "Seleziona tributi (ESC annulla)", 20);
                    trib.setFillColor(sf::Color(255,200,90,240));
                    trib.setOutlineColor(sf::Color(0,0,0,180));
                    trib.setOutlineThickness(2.f);
                    trib.setPosition(sf::Vector2f(15.f, 66.f));
                    window.draw(trib);
                }
                if(over>0){
                    sf::Text warn(detailFont, "Limite mano superato: scarterai in End Phase", 20);
                    warn.setFillColor(sf::Color(255,80,80,240));
                    warn.setOutlineColor(sf::Color(0,0,0,200));
                    warn.setOutlineThickness(2.f);
                    warn.setPosition(sf::Vector2f(15.f, 38.f));
                    window.draw(warn);
                }
                if(ssChoiceActive){
                    sf::Text prompt(detailFont, "Evocazione Speciale: A = Attacco, D = Difesa (mai coperto)", 22);
                    prompt.setFillColor(sf::Color(255,230,140,245));
                    prompt.setOutlineColor(sf::Color(0,0,0,200));
                    prompt.setOutlineThickness(2.f);
                    prompt.setPosition(sf::Vector2f(15.f, 94.f));
                    window.draw(prompt);
                }
                if(chooseActivateOrSetST){
                    sf::Text prompt(detailFont, "Magia/Trappola: A = Attiva, S = Setta", 20);
                    prompt.setFillColor(sf::Color(255,230,140,245));
                    prompt.setOutlineColor(sf::Color(0,0,0,200));
                    prompt.setOutlineThickness(2.f);
                    prompt.setPosition(sf::Vector2f(15.f, 122.f));
                    window.draw(prompt);
                }
                // Suggerimenti e feedback per la fase Battle
                if(game->getTurn().getPhase() == GamePhase::Battle){
                    sf::Text hint(detailFont, "Battle: clicca un tuo mostro (rossi = gia' usati), tasto A = attacco diretto", 20);
                    hint.setFillColor(sf::Color(200,230,255,235));
                    hint.setOutlineColor(sf::Color(0,0,0,180));
                    hint.setOutlineThickness(2.f);
                    hint.setPosition(sf::Vector2f(15.f, 38.f));
                    window.draw(hint);
                }
                if(!battleFeedbackMsg.empty()){
                    float elapsed = battleFeedbackClock.getElapsedTime().asSeconds();
                    if(elapsed < BATTLE_FEEDBACK_DURATION){
                        sf::Text fb(detailFont, battleFeedbackMsg, 22);
                        fb.setFillColor(sf::Color(255,200,90,235));
                        fb.setOutlineColor(sf::Color(0,0,0,180));
                        fb.setOutlineThickness(2.f);
                        sf::FloatRect b = fb.getLocalBounds();
                        fb.setPosition(sf::Vector2f((windowSize.x - b.size.x)/2.f, 70.f));
                        window.draw(fb);
                    } else {
                        battleFeedbackMsg.clear();
                    }
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

        // Disegno overlay Game Over (sopra tutto)
        if(gameOverActive){
            sf::RectangleShape overlay(sf::Vector2f(windowSize.x, windowSize.y));
            overlay.setFillColor(sf::Color(0,0,0,160));
            window.draw(overlay);

            // Pannello centrale
            sf::Vector2f panelSize(720.f, 320.f);
            sf::Vector2f center(windowSize.x/2.f, windowSize.y/2.f);
            sf::Vector2f pos(center.x - panelSize.x/2.f, center.y - panelSize.y/2.f);
            sf::RectangleShape panel(panelSize);
            panel.setPosition(pos);
            panel.setFillColor(sf::Color(25,25,25,235));
            panel.setOutlineColor(sf::Color(255,200,90));
            panel.setOutlineThickness(3.f);
            window.draw(panel);

            // Titolo e messaggi
            std::string titleStr = "Game Over";
            if(game && game->isGameOver()){
                auto w = game->getWinnerIndex();
                if(w.has_value()){
                    titleStr = std::string("Vittoria: Player ") + std::to_string(w.value()+1);
                }
            }
            sf::Text title(detailFont, titleStr, 54);
            title.setFillColor(sf::Color(255,230,140));
            title.setOutlineColor(sf::Color(0,0,0,200));
            title.setOutlineThickness(3.f);
            sf::FloatRect tB = title.getLocalBounds();
            title.setOrigin(sf::Vector2f(tB.position.x + tB.size.x/2.f, tB.position.y + tB.size.y/2.f));
            title.setPosition(sf::Vector2f(center.x, pos.y + 110.f));
            window.draw(title);

            sf::Text hint(detailFont, "Premi SPACE per tornare alla Home", 26);
            hint.setFillColor(sf::Color(240,240,240,240));
            hint.setOutlineColor(sf::Color(0,0,0,200));
            hint.setOutlineThickness(2.f);
            sf::FloatRect hB = hint.getLocalBounds();
            hint.setOrigin(sf::Vector2f(hB.position.x + hB.size.x/2.f, hB.position.y + hB.size.y/2.f));
            hint.setPosition(sf::Vector2f(center.x, pos.y + panelSize.y - 70.f));
            window.draw(hint);
        }

        window.display();
    }
    return 0;
}