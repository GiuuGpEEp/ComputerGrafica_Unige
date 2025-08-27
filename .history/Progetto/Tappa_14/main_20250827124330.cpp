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
#include "Config.h"
#include "GameLogic/Game/Effects/cards/WhiteStoneLegendEffect.h"
#include "GameLogic/Game/Effects/cards/WhiteStoneAncientsEffect.h"
// Effetti di attivazione
#include "GameLogic/Game/Effects/cards/ReliquarioDelDragoEffect.h"
#include "GameLogic/Game/Effects/cards/PiantoDiArgentoEffect.h"
// AI controller
#include "GameLogic/AI/AIController.h"
#include "RenderUtils.h"

#define P1 1 
#define P2 2
#define DECK_SIZE 30 // Numero di carte nel deck
#define HAND_MAXSIZE 7 
#define CARD_MAXOFFSET 50.f
#define MOVEMENT_SPEED 2000.f


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
    
    // Stato per la mano iniziale alternata (P1, P2, P1, P2...) fino a 5 carte ciascuno
    bool openingAltDrawActive = false;
    int openingNextPlayer = 0;       // prossimo giocatore a pescare (0=bottom, 1=top)
    bool openingDrawInFlight = false; // c'è un'animazione di pescata in corso per l'apertura
    int openingCurrentPlayer = -1;    // 0/1: a chi è destinata la pescata in corso

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
    sf::RenderWindow window(sf::VideoMode(windowSize), "Progetto Tappa 14");

    // TextureManager per tutte le texture
    TextureManager textureManager;

    // Precarica e costruisci una mappa key->texture per usare nomi simbolici nel main
    auto tex = AppConfig::buildTextureMap(textureManager);

    // Carico il font per le etichette delle carte
    sf::Font detailFont;
    AppConfig::loadDetailFont(detailFont, AppConfig::fontPath);

    // Crea il campo di gioco con dimensioni dinamiche
    Field field(
        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::FieldBackground),
        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::MonsterText),
        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::SpellTrap),
        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::Deck),
        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::Graveyard),
        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::ExtraDeck),
        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::FieldSpell),
        windowSize
    );        

    // Calcola le dimensioni per le carte del deck (leggermente più piccole dello slot)
    sf::Vector2f slotSize = calculateSlotSize(windowSize);
    float deckScaleFactor = 0.9f; 
    sf::Vector2f deckCardSize(slotSize.x * deckScaleFactor, slotSize.y * deckScaleFactor);

    // Posizione degli slot del deck principale e dell'extra deck (giocatore 1 in basso, giocatore 2 in alto)
    sf::Vector2f deckSlotPos = field.getSlotPosition(Type::Deck, P1); 
    sf::Vector2f extraDeckSlotPos = field.getSlotPosition(Type::Extra, P1); // usato per disegnare lo stack dell'extra deck (P1)
    sf::Vector2f deckSlotPosTop = field.getSlotPosition(Type::Deck, P2); 
    sf::Vector2f extraDeckSlotPosTop = field.getSlotPosition(Type::Extra, P2);


    // --- INTEGRAZIONE JSON ---
    // ========== CREA E INIZIALIZZA IL RESOURCE MANAGER ==========
    ResourceManager resourceManager(textureManager, AppConfig::resourceBase);

    if (!resourceManager.initialize(
            deckSlotPos, extraDeckSlotPos, deckCardSize, slotSize,
            AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack))) {
        std::cerr << "ERRORE: Impossibile inizializzare ResourceManager!" << std::endl;
        std::cerr << "Il gioco continuerà con un deck vuoto." << std::endl;
    }
    
    Deck deck = resourceManager.getDeckByName(""); 
    // Deck UI del giocatore 2 (in alto)
    Deck opponentDeck = resourceManager.getDeckByName("");
    
    // Overlay Extra Deck
    ExtraDeckOverlay extraOverlay(windowSize, slotSize, textureManager);
    GraveyardOverlay graveyardOverlay(windowSize, slotSize, textureManager);

    // Creo le carte in mano, inizialmente il vettore raffigurante le carte in mano è vuoto
    std::vector<Card>* handPtr = nullptr; // riferimento alla mano VISIBILE (sempre Player assoluto 0)
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
    HomePage homeScreen(window, detailFont, windowSize,
        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::HomeScreen), deck.getDeckName());

    DeckSelectionScreen deckSelectionScreen(window, textureManager, resourceManager,
        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::DeckSelectionScreen), windowSize, detailFont);
    std::string selectedDeckName = "";
    bool deckSelected = false;

    // Logica di gioco principale (creata dopo la selezione del deck)
    std::unique_ptr<Game> game; // gestisce giocatori, turni, fasi
    DrawController drawController; 
    DiscardController discardController; 
    AIController aiP2{1}; // AI sul Player 2 (alto)

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

    // Prompt attivazione istantanea (Trap/Quick) nel turno avversario
    bool responsePromptActive = false;
    int responseOwner = -1; // 0/1 assoluto del proprietario con priorita'
    std::vector<size_t> responseActivables; // indici di zona ST attivabili
    size_t responseSelected = 0;
    // Stato overlay scelta invio dal Deck (per effetti come Reliquario)
    bool deckSendChoiceActive = false;
    int deckSendOwner = -1;
    std::vector<Card> deckSendCandidates;
    size_t deckSendSelected = 0;
    
    auto rebuildResponseActivables = [&](){
        responseActivables.clear();
        if (!game) return;
        // Se non definito un owner, assumiamo il non-turn player
        int cur = game->getTurn().getCurrentPlayerIndex();
        if (responseOwner < 0) responseOwner = 1 - cur;
        for (size_t i = 0; i < Game::ST_ZONE_SIZE; ++i) {
            if (game->canActivateSetSpellTrapFor(responseOwner, i)) {
                responseActivables.push_back(i);
            }
        }
        if (responseSelected >= responseActivables.size()) responseSelected = 0;
    };
    
    auto openResponsePromptIfAny = [&](){
        if (!game) return;
        int cur = game->getTurn().getCurrentPlayerIndex();
        // Mostra il prompt solo nel turno dell'avversario (cur == 1) e solo per le carte del giocatore in basso (owner assoluto 0)
        if (cur != 1) return;
        responseOwner = 0;
        rebuildResponseActivables();
        if (!responseActivables.empty()) {
            responsePromptActive = true;
            responseSelected = 0;
            // Rendi modale: interrompi eventuali drag/azioni in corso
            isDragging = false; isPotentialDrag = false; draggingCardIndex.reset(); potentialDragCardIndex.reset();
        }
    };
    
    auto closeResponsePrompt = [&](){
        responsePromptActive = false; responseActivables.clear(); responseSelected = 0; responseOwner = -1;
    };
    
    // Sincronizza la rappresentazione grafica della zona mostri (Player1) con lo stato logico del Game
    auto syncMonsterZoneToField = [&](){
        if (!game) return;
        // In basso (P1) deve sempre mostrare la zona del Giocatore 0, in alto (P2) quella del Giocatore 1,
        // anche quando è il turno del Giocatore 1. Poiché Game espone solo le viste current/opponent,
        // ricaviamo le zone assolute dall'indice del giocatore corrente.
        int cur = game->getTurn().getCurrentPlayerIndex();
        const auto &p1Zone = (cur == 0) ? game->getMonsterZone() : game->getOpponentMonsterZone();
        const auto &p2Zone = (cur == 0) ? game->getOpponentMonsterZone() : game->getMonsterZone();

        // Reset occupazione visiva degli slot Mostro per P1 e P2
        for (int s = 0; s < 3; ++s) {
            sf::Vector2f pos1 = field.getSlotPosition(Type::Monster, P1, s);
            if (auto sl = field.getSlotByPosition(pos1)) sl->setOccupied(false);
            sf::Vector2f pos2 = field.getSlotPosition(Type::Monster, P2, s);
            if (auto sl2 = field.getSlotByPosition(pos2)) sl2->setOccupied(false);
        }

        fieldCards.clear();
        float fieldCardScale = RenderUtils::FieldScale;
        sf::Vector2f fieldCardSize(slotSize.x * RenderUtils::FieldScale, slotSize.y * RenderUtils::FieldScale);
        for (size_t i = 0; i < p1Zone.size(); ++i) {
            Card c = p1Zone[i]; // copia per il rendering
            // Se la carta è scoperta sul campo P1, ripristina la texture originale frontale
            bool isFD = game->isFaceDownAt(0, i);
            if (!isFD) { RenderUtils::applyFrontTextureIfAny(c, textureManager); }
            int slotIdx = game->getMonsterSlotIndexAt(0, i);
            if (slotIdx < 0) slotIdx = static_cast<int>(i);
            sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, P1, slotIdx);
            RenderUtils::placeCentered(c, slotPos, slotSize, fieldCardScale, fieldCardScale);
            fieldCards.push_back(c);
            // marca occupato lo slot
            if (auto sl = field.getSlotByPosition(slotPos)) sl->setOccupied(true);
        }
        // Avversario (P2 in alto)
        oppFieldCards.clear();
        for (size_t i = 0; i < p2Zone.size(); ++i) {
            Card c = p2Zone[i];
            // Il lato alto è il Giocatore 1: se scoperta, usa texture originale
            bool isFD2 = game->isFaceDownAt(1, i);
            if (!isFD2) { RenderUtils::applyFrontTextureIfAny(c, textureManager); }
            int slotIdx2 = game->getMonsterSlotIndexAt(1, i);
            if (slotIdx2 < 0) slotIdx2 = static_cast<int>(i);
            sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, P2, slotIdx2);
            RenderUtils::placeCentered(c, slotPos, slotSize, fieldCardScale, fieldCardScale);
            oppFieldCards.push_back(c);
            if (auto sl = field.getSlotByPosition(slotPos)) sl->setOccupied(true);
        }
        // Corregge un eventuale indice selezionato non più valido
        if (selectedCardIsOnField && selectedCardIndex.has_value() && selectedCardIndex.value() >= fieldCards.size()) {
            selectedCardIndex.reset();
            selectedCardIsOnField = false;
        }
    };

    // Sincronizza M/T e Field Spell con lo stato logico
    auto syncSpellTrapZones = [&](){
        if (!game) return;
        int cur = game->getTurn().getCurrentPlayerIndex();
        const auto &p1ST = (cur==0) ? game->getSpellTrapZone() : game->getOpponentSpellTrapZone();
        const auto &p2ST = (cur==0) ? game->getOpponentSpellTrapZone() : game->getSpellTrapZone();

        // Reset occupazione slot ST e FieldSpell
        for (int s = 0; s < 3; ++s) {
            if (auto sl = field.getSlotByPosition(field.getSlotPosition(Type::SpellTrap, P1, s))) sl->setOccupied(false);
            if (auto sl = field.getSlotByPosition(field.getSlotPosition(Type::SpellTrap, P2, s))) sl->setOccupied(false);
        }
        if (auto sl = field.getSlotByPosition(field.getSlotPosition(Type::FieldSpell, P1))) sl->setOccupied(false);
        if (auto sl = field.getSlotByPosition(field.getSlotPosition(Type::FieldSpell, P2))) sl->setOccupied(false);

        p1STCards.clear(); p2STCards.clear();
        // P1 in basso = assoluto 0
        for (size_t i = 0; i < p1ST.size(); ++i) {
            Card c = p1ST[i];
            bool isFD = game->isSpellTrapFaceDownAt(0, i);
            if (!isFD) { RenderUtils::applyFrontTextureIfAny(c, textureManager); }
            sf::Vector2f slotPos = field.getSlotPosition(Type::SpellTrap, P1, (int)i);
            RenderUtils::placeCentered(c, slotPos, slotSize, RenderUtils::STScale, RenderUtils::STScale);
            p1STCards.push_back(c);
            if (auto sl = field.getSlotByPosition(slotPos)) sl->setOccupied(true);
        }
        // P2 in alto = assoluto 1
        for (size_t i = 0; i < p2ST.size(); ++i) {
            Card c = p2ST[i];
            bool isFD = game->isSpellTrapFaceDownAt(1, i);
            if (!isFD) { RenderUtils::applyFrontTextureIfAny(c, textureManager); }
            sf::Vector2f slotPos = field.getSlotPosition(Type::SpellTrap, P2, (int)i);
            RenderUtils::placeCentered(c, slotPos, slotSize, RenderUtils::STScale, RenderUtils::STScale);
            p2STCards.push_back(c);
            if (auto sl = field.getSlotByPosition(slotPos)) sl->setOccupied(true);
        }
        // Field Spell slot singolo per lato
        p1FieldSpellCard = game->getFieldSpellOf(0);
        if (p1FieldSpellCard.has_value()) {
            Card &c = p1FieldSpellCard.value();
            RenderUtils::applyFrontTextureIfAny(c, textureManager);
            sf::Vector2f pos = field.getSlotPosition(Type::FieldSpell, P1);
            RenderUtils::placeCentered(c, pos, slotSize, RenderUtils::FieldSpellScale, RenderUtils::FieldSpellScale);
            if (auto sl = field.getSlotByPosition(pos)) sl->setOccupied(true);
        }
        p2FieldSpellCard = game->getFieldSpellOf(1);
        if (p2FieldSpellCard.has_value()) {
            Card &c = p2FieldSpellCard.value();
            RenderUtils::applyFrontTextureIfAny(c, textureManager);
            sf::Vector2f pos = field.getSlotPosition(Type::FieldSpell, P2);
            RenderUtils::placeCentered(c, pos, slotSize, RenderUtils::FieldSpellScale, RenderUtils::FieldSpellScale);
            if (auto sl = field.getSlotByPosition(pos)) sl->setOccupied(true);
        }
    };
    // Funzione/lambda per resettare completamente la partita quando si torna alla Home
    // Cancella mano, carte sul campo e animazioni
    auto resetMatch = [&](){
        if(handPtr) handPtr->clear();
        fieldCards.clear();
        oppFieldCards.clear();
        // Svuota anche le cache di rendering di Magie/Trappole e Field Spell per evitare residui
        p1STCards.clear();
        p2STCards.clear();
        p1FieldSpellCard.reset();
        p2FieldSpellCard.reset();
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
        // Reset apertura alternata
        openingAltDrawActive = false;
        openingDrawInFlight = false;
        openingCurrentPlayer = -1;
        openingNextPlayer = 0;
        // Ricrea il deck (se è già stato scelto un nome) così le carte ritornano nel mazzo
        if(!selectedDeckName.empty()){
            deck = resourceManager.getDeckByName(selectedDeckName);
            opponentDeck = resourceManager.getDeckByName(selectedDeckName);
        } else {
            deck = resourceManager.getDeckByName("");
            opponentDeck = resourceManager.getDeckByName("");
        }
        // Reimposta posizioni e stato fade dei deck
        deck.resetDeckCardPositions(
            deckSlotPos, deckCardSize, slotSize,
            AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack));
        deck.resetAnimation();
        opponentDeck.resetDeckCardPositions(
            deckSlotPosTop, deckCardSize, slotSize,
            AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack));
        opponentDeck.resetAnimation();
        
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

    FieldLoadingAnimation fieldLoadingAnim = FieldLoadingAnimation(
        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::HomeScreen),
        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::FieldBackground),
        windowSize);

    // Popup conferma ritorno alla Home
    bool returnPopupActive = false;
    // Overlay Game Over (vittoria/sconfitta)
    bool gameOverActive = false;

    while (window.isOpen()) {

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
                        switch(RenderUtils::confirmReturnHomeHitTest(windowSize, sf::Mouse::getPosition(window))){
                            case RenderUtils::ConfirmHit::Yes:
                                gamestate = GameState::HomeScreen;
                                returnPopupActive = false;
                                resetMatch();
                                break;
                            case RenderUtils::ConfirmHit::No:
                                returnPopupActive = false;
                                break;
                            default: break;
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
                if(keyPressed->code == sf::Keyboard::Key::N && gamestate == GameState::Playing && game && !selectingTributes && !ssChoiceActive && !responsePromptActive){
                    game->advancePhase();
                    std::cout << "Fase -> " << phaseToString(game->getTurn().getPhase()) << std::endl;
                    // Se e' il turno dell'avversario (top), valuta apertura prompt
                    openResponsePromptIfAny();
                    // Reset della selezione tributi fuori dalle fasi Main
                    if(game->getTurn().getPhase() != GamePhase::Main1 && game->getTurn().getPhase() != GamePhase::Main2){
                        selectingTributes = false; selectedTributes.clear(); tributesNeeded = 0;
                    }
                    // Abilita la UI di battaglia solo in Battle
                    attackSelectionActive = (game->getTurn().getPhase() == GamePhase::Battle);
                    if(!attackSelectionActive){ selectedAttackerIndex.reset(); }
                }
                // Forza fine turno (debug)
                if(keyPressed->code == sf::Keyboard::Key::T && gamestate == GameState::Playing && game && !selectingTributes && !ssChoiceActive && !responsePromptActive){
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
                    sf::Vector2f(0.f,0.f), sf::Vector2f(0.f,0.f),
                    AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack),
                    Type::Monster, Attribute::None, 4, {});
                    game->debugAddMonsterToOpponent(dbg);
                    syncMonsterZoneToField();
                }
                // Tasto rapido per attacco diretto (disabilitato durante selezione tributi)
                if(keyPressed->code == sf::Keyboard::Key::A && gamestate == GameState::Playing && game && !responsePromptActive){
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
                // Navigazione con click nell'overlay di scelta invio dal Deck
                if(deckSendChoiceActive && but == sf::Mouse::Button::Left){
                    auto hit = RenderUtils::deckSendHitIndex(windowSize, slotSize, deckSendCandidates.size(), sf::Mouse::getPosition(window));
                    if(hit.has_value()) deckSendSelected = hit.value();
                }
                // Tasto destro nella deck selection: fade out
                    if(but == sf::Mouse::Button::Right && gamestate == GameState::DeckSelection) {
                    std::cout << "Right click -> Fade out DeckSelection" << std::endl;
                    if(!deckSelectionScreen.isFading()) deckSelectionScreen.startFadeOut(0.4f);
                }
                    // Targeting in battaglia: click destro su un mostro avversario per selezionarlo (disabilitato durante selezione tributi)
                    if(but == sf::Mouse::Button::Right && gamestate == GameState::Playing && game && !selectingTributes && !responsePromptActive && game->getTurn().getPhase() == GamePhase::Battle){
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
                    if(!selectingTributes && !responsePromptActive){
                        extraOverlay.startHold(static_cast<sf::Vector2f>(mousePos), sf::FloatRect(extraDeckSlotPos, slotSize), gamestate);
                        sf::Vector2f gyPos = field.getSlotPosition(Type::Graveyard, P1);
                        bool gyHasCards = false;
                        if(game){ gyHasCards = !game->getGraveyard().empty(); }
                        graveyardOverlay.startHold(static_cast<sf::Vector2f>(mousePos), sf::FloatRect(gyPos, slotSize), gamestate, gyHasCards);
                    }
                    
                    // Gestione click su overlay (disabilitati durante selezione tributi)
                    if(!selectingTributes && !responsePromptActive){
                        extraOverlay.handleMousePress(static_cast<sf::Vector2f>(mousePos), deck.getExtraCards());
                        if(game){
                            graveyardOverlay.handleMousePress(static_cast<sf::Vector2f>(mousePos), game->getGraveyard());
                        }
                    }

                    if(!responsePromptActive && gamestate == GameState::HomeScreen && homeScreen.getGiocaBounds().contains(initialMousePos)) {
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
                            opponentDeck.clearShuffleAnimationAdvanced();
                            // Allinea anche il deck avversario per la resa visiva (stesso contenuto del deck selezionato)
                            opponentDeck = resourceManager.getDeckByName(selectedDeckName);
                            // Ricrea sempre un nuovo Game (anche senza passare di nuovo dalla DeckSelection)
                            Player p1("Player1", deck);
                            // Usa lo stesso mazzo selezionato anche per Player 2 (copiato)
                            Player p2("Player2", opponentDeck);
                            game = std::make_unique<Game>(p1, p2);
                            aiP2.attachGame(game.get());
                            // Pulisci le viste ST/FieldSpell prima che il nuovo Game parta
                            p1STCards.clear();
                            p2STCards.clear();
                            p1FieldSpellCard.reset();
                            p2FieldSpellCard.reset();
                            // Registra effetti carta (assicurati anche nel flusso "Gioca!" da Home)
                            game->registerEffectForCardName("Pietra Bianca della Leggenda", std::make_unique<WhiteStoneLegendEffect>());
                            game->registerEffectForCardName("Pietra Bianca degli Antichi", std::make_unique<WhiteStoneAncientsEffect>());
                            // Registra effetti di attivazione per Magie
                            game->registerActivationForCardName("Reliquario del Drago", std::make_unique<ReliquarioDelDragoEffect>());
                            game->registerActivationForCardName("Pianto di Argento", std::make_unique<PiantoDiArgentoEffect>());
                            game->attachExternalDeck(&deck);
                            game->attachDrawController(&drawController);
                            // Mantieni la mano visibile sempre del giocatore assoluto 0
                            if(game){
                                int cur = game->getTurn().getCurrentPlayerIndex();
                                handPtr = (cur==0) ? &game->current().getHand() : &game->opponent().getHand();
                                if(handPtr) handPtr->clear();
                            }
                            // Le sottoscrizioni agli eventi verranno impostate dopo l'avvio del gioco
                            gamestate = GameState::FieldLoading; // stato intermedio con animazione
                        }
                    } 

                    if(!responsePromptActive && gamestate == GameState::HomeScreen && homeScreen.getSelectionBounds().contains(initialMousePos)) {
                        std::cout << "Cliccato su Selezione Mazzo!" << std::endl;
                        gamestate = GameState::DeckSelection;
                        deckSelectionScreen.startFadeIn(0.5f);
                    }

                    if(!responsePromptActive && gamestate == GameState::DeckSelection){
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

                    if (!responsePromptActive && isDragging && draggingCardIndex.has_value()) {
                        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                        bool hasFreeSlotUnderMouse = false;
                        if(handPtr){
                            hasFreeSlotUnderMouse = findSlotPosition(mousePos, (*handPtr)[draggingCardIndex.value()], field, slotSize).has_value();
                        }
                        bool canOpenSummonChoiceForTribute = false;
                        if(game && handPtr && !selectingTributes){
                            size_t idx = draggingCardIndex.value();
                            if((*handPtr)[idx].getType() == Type::Monster){
                                int need = game->requiredTributesFor((*handPtr)[idx]);
                                if(need > 0){
                                    // il giocatore di turno deve avere abbastanza mostri da tributare
                                    const auto &mz = game->getMonsterZone();
                                    if(mz.size() >= static_cast<size_t>(need)){
                                        canOpenSummonChoiceForTribute = true;
                                    }
                                }
                            }
                        }
                        if (hasFreeSlotUnderMouse || canOpenSummonChoiceForTribute) {
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
                    // ESC chiude il prompt di risposta ma non blocca altre azioni
                    if(responsePromptActive){
                        closeResponsePrompt();
                        // Nota: l'AI non avanza in questo frame perche' l'update e' gia' condizionato da !responsePromptActive
                        // non fare continue, lascia proseguire con altre deselezioni
                    }
                    // ESC chiude overlay scelta DeckSend
                    if(deckSendChoiceActive){
                        if(game){ game->cancelPendingSendFromDeck(); }
                        deckSendChoiceActive = false; deckSendCandidates.clear(); deckSendSelected = 0; deckSendOwner = -1;
                        continue; // non proseguire con altre deselezioni
                    }
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
                    }
                }
                // Conferma attivazione dal prompt risposta
                if(responsePromptActive && (keyPressed->code == sf::Keyboard::Key::Enter)){
                    if(game && responseSelected < responseActivables.size()){
                        size_t zoneIdx = responseActivables[responseSelected];
                        bool ok = game->activateSetSpellTrapFor(responseOwner, zoneIdx);
                        if(ok){
                            battleFeedbackMsg = "Carta attivata"; battleFeedbackClock.restart();
                            syncSpellTrapZones();
                        } else {
                            battleFeedbackMsg = "Impossibile attivarla ora"; battleFeedbackClock.restart();
                        }
                        closeResponsePrompt();
                    }
                }
                // Conferma scelta DeckSend con Invio
                if(deckSendChoiceActive && (keyPressed->code == sf::Keyboard::Key::Enter)){
                    if(game){
                        if(game->resolvePendingSendFromDeck(deckSendSelected)){
                            deckSendChoiceActive = false; deckSendCandidates.clear(); deckSendSelected = 0; deckSendOwner = -1;
                            // feedback opzionale
                            battleFeedbackMsg = "Carta inviata al Cimitero"; battleFeedbackClock.restart();
                        } else {
                            battleFeedbackMsg = "Selezione non valida"; battleFeedbackClock.restart();
                        }
                    }
                }
                // Scelta Evocazione/Posizionamento
                if(chooseSummonOrSet && game && pendingHandIndexForPlay.has_value() && !responsePromptActive){
                    if(keyPressed->code == sf::Keyboard::Key::E){
                        // Calcola lo slot sotto il mouse al momento della scelta
                        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                        auto slotIdxOpt = findMonsterSlotIndexUnderMouse(mousePos, field, slotSize);
                        bool ok = false;
                        // Tenta sempre: se richiede tributi, il Game avviera' il flusso anche con zona piena
                        if(slotIdxOpt.has_value()) ok = game->tryNormalSummonToSlot(pendingHandIndexForPlay.value(), static_cast<size_t>(*slotIdxOpt));
                        else ok = game->tryNormalSummon(pendingHandIndexForPlay.value());
                        if(ok){
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
                        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                        auto slotIdxOpt = findMonsterSlotIndexUnderMouse(mousePos, field, slotSize);
                        bool ok = false;
                        // Stesso comportamento: consenti il flusso tributi anche con campo pieno
                        if(slotIdxOpt.has_value()) ok = game->tryNormalSetToSlot(pendingHandIndexForPlay.value(), static_cast<size_t>(*slotIdxOpt));
                        else ok = game->tryNormalSet(pendingHandIndexForPlay.value());
                        if(ok){
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
                        if(!responsePromptActive){
                            if(selectedCardIsOnField && selectedCardIndex.has_value()){
                                if(game->togglePosition(selectedCardIndex.value())){
                                    battleFeedbackMsg = "Posizione cambiata";
                                    battleFeedbackClock.restart();
                                }
                            }
                        }
                    } else if(keyPressed->code == sf::Keyboard::Key::X && !responsePromptActive){
                        // Attiva una tua M/T settata sotto al mouse (solo nel tuo turno)
                        sf::Vector2i mpos = sf::Mouse::getPosition(window);
                        bool triggered = false; // kept for future use; has no effect when prompt is open
                        int cur = game->getTurn().getCurrentPlayerIndex();
                        if(cur == 0){
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
                        }
                    } else if(responsePromptActive){
                        // Navigazione nel prompt: frecce su/giù (o sinistra/destra)
                        if(keyPressed->code == sf::Keyboard::Key::Up || keyPressed->code == sf::Keyboard::Key::Left){
                            if(responseSelected > 0) responseSelected--;
                        } else if(keyPressed->code == sf::Keyboard::Key::Down || keyPressed->code == sf::Keyboard::Key::Right){
                            if(responseSelected + 1 < responseActivables.size()) responseSelected++;
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
            opponentDeck = resourceManager.getDeckByName(selectedDeckName);

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
            // Usa lo stesso deck anche per Player 2 (copiato)
            Player p2("Player2", opponentDeck);
            game = std::make_unique<Game>(p1, p2);
            aiP2.attachGame(game.get());
            // Pulisci le viste ST/FieldSpell quando si crea un nuovo Game dalla Deck Selection
            p1STCards.clear();
            p2STCards.clear();
            p1FieldSpellCard.reset();
            p2FieldSpellCard.reset();
            // Registra effetti base (per tipo). Iniziamo con "Pietra Bianca della Leggenda" (search BEWD on GY)
            game->registerEffectForCardName("Pietra Bianca della Leggenda", std::make_unique<WhiteStoneLegendEffect>());
            game->registerEffectForCardName("Pietra Bianca degli Antichi", std::make_unique<WhiteStoneAncientsEffect>());
            game->registerActivationForCardName("Reliquario del Drago", std::make_unique<ReliquarioDelDragoEffect>());
            game->registerActivationForCardName("Pianto di Argento", std::make_unique<PiantoDiArgentoEffect>());
            game->attachExternalDeck(&deck);
            game->attachDrawController(&drawController);
            // Mano visibile = assoluto 0
            if(game){
                int cur = game->getTurn().getCurrentPlayerIndex();
                handPtr = (cur==0) ? &game->current().getHand() : &game->opponent().getHand();
                handPtr->clear();
            }
            deckSelected = false; // Evita di rieseguire ogni frame    
        }
        
        // Gestione dell'animazione FieldLoading
        if(gamestate == GameState::FieldLoading && !fieldLoadingAnim.hasStarted()){
            fieldLoadingAnim.start(
                AppConfig::findTextureInMap(tex, AppConfig::TextureKey::HomeScreen),
                AppConfig::findTextureInMap(tex, AppConfig::TextureKey::FieldBackground),
                windowSize);
        }
    
        if(gamestate == GameState::FieldLoading){
            fieldLoadingAnim.update(deltaTime);
            if(fieldLoadingAnim.isFinished()){
                std::cout << "Transizione completata: FieldLoading -> FieldVisible" << std::endl;
                gamestate = GameState::FieldVisible;   
            }    
        }

    // Controlla se il mouse si è mosso abbastanza e nella direzione corretta per attivare il dragging (bloccato durante la selezione tributi o overlay attivi)
    // Blocca il drag quando il prompt di risposta è aperto (prompt modale)
    if (isPotentialDrag && potentialDragCardIndex.has_value() && !selectingTributes && !deckSendChoiceActive && !responsePromptActive) {
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
            // Anima anche il deck dell'avversario per mantenere la visibilità dopo lo shuffle
            opponentDeck.animate(deltaTime);
            if(deck.isAnimationFinished()) {
                deck.setAnimationFinished();
                    
                // Avvia le shuffle animation avanzate una sola volta (entrambi i deck)
                if (!shuffleStarted) {
                    deck.startShuffleAnimationAdvanced(deckSlotPos, deckCardSize);
                    opponentDeck.startShuffleAnimationAdvanced(deckSlotPosTop, deckCardSize);
                    shuffleStarted = true;
                }
                // Aggiorna le shuffle animation
                deck.updateShuffleAnimationAdvanced(deltaTime);
                opponentDeck.updateShuffleAnimationAdvanced(deltaTime);
                // Quando l'animazione è finita, mischia effettivamente il deck e passa allo stato Playing
                if (deck.isShuffleAnimationAdvancedFinished()) {
                    deck.shuffle();
                    opponentDeck.shuffle();
                    deck.resetDeckCardPositions(
                        deckSlotPos, deckCardSize, slotSize,
                        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack));
                    opponentDeck.resetDeckCardPositions(    
                        deckSlotPosTop, deckCardSize, slotSize,
                        AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack));
                    std::cout << "Passaggio allo stato Playing..." << std::endl;        
                    gamestate = GameState::Playing;
                        
                    if(game && !game->isStarted()){
                        game->start();
                        // Avvio della mano iniziale alternata: nessuna pescata automatica, gestiamo noi 1 a 1.
                        openingAltDrawActive = true;
                        openingNextPlayer = game->getStartingPlayerIndex();
                        openingDrawInFlight = false;
                        openingCurrentPlayer = -1;
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
                            // Aggiorna il puntatore alla mano visibile (sempre player assoluto 0)
                            if(game){
                                int cur2 = game->getTurn().getCurrentPlayerIndex();
                                handPtr = (cur2==0) ? &game->current().getHand() : &game->opponent().getHand();
                                // riallinea layout della mano visibile
                                if(handPtr){ updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE); }
                            }
                            // Alla partenza turno, se l'attuale non-turno e' P1 (bottom), apri prompt se ci sono risposte
                            openResponsePromptIfAny();
                        });
                        
                        // Ad ogni cambio fase nel turno avversario, riproponi il prompt di risposta se ci sono carte attivabili
                        game->events().subscribe(GameEventType::PhaseChange, [&](){
                            openResponsePromptIfAny();
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
                            // Apri il prompt di risposta per il giocatore non di turno, se ha ST attivabili
                            openResponsePromptIfAny();
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
                            // Riallinea anche lo stack del Deck UI (puo' essere cambiato da effetti)
                            deck.resetDeckCardPositions(
                                deckSlotPos, deckCardSize, slotSize,
                                AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack));
                            opponentDeck.resetDeckCardPositions(
                                deckSlotPosTop, deckCardSize, slotSize,
                                AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack));
                            // Aggiorna layout della mano se è cambiata (es. aggiunta carta da un effetto)
                            if(handPtr){
                                // Garantisci che le carte in mano mostrino sempre il fronte
                                for(auto &c : *handPtr){
                                    RenderUtils::applyFrontTextureIfAny(c, textureManager);
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
                                if(responsePromptActive){ rebuildResponseActivables(); if(responseActivables.empty()) closeResponsePrompt(); }
                            });
                            game->events().subscribe(GameEventType::TrapActivated, [&](){
                                battleFeedbackMsg = "Trappola attivata"; battleFeedbackClock.restart();
                                syncSpellTrapZones();
                                if(responsePromptActive){ rebuildResponseActivables(); if(responseActivables.empty()) closeResponsePrompt(); }
                            });
                            // Richiesta scelta invio dal Deck (es. Reliquario)
                            game->events().subscribe(GameEventType::DeckSendChoiceRequested, [&](){
                                if(!game) return;
                                deckSendChoiceActive = true;
                                deckSendOwner = game->getTurn().getCurrentPlayerIndex();
                                // Raccogli candidati Drago dal Deck del proprietario
                                const Deck &d = game->getDeckOf(deckSendOwner);
                                deckSendCandidates = d.collectWhere([](const Card& c){
                                    const auto &feats = c.getFeatures();
                                    return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
                                });
                                deckSendSelected = 0;
                                if(deckSendCandidates.empty()){
                                    // Nulla da scegliere: annulla e notifica
                                    game->cancelPendingSendFromDeck();
                                    deckSendChoiceActive = false; deckSendOwner = -1;
                                    battleFeedbackMsg = "Nessun Drago nel Deck"; battleFeedbackClock.restart();
                                    return;
                                }
                                // Ferma input conflittuali
                                isDragging = false; isPotentialDrag = false; draggingCardIndex.reset(); potentialDragCardIndex.reset();
                            });
                            // Richiesta di scelta per Evocazione Speciale (Attacco o Difesa, mai coperto)
                            game->events().subscribe(GameEventType::SpecialSummonChoiceRequested, [&](){
                                std::cout << "[Event] SpecialSummonChoiceRequested" << std::endl;
                                ssChoiceOwner = game->getTurn().getCurrentPlayerIndex();
                                // Se la scelta spetta al Player 2 (in alto), risolvi automaticamente senza aprire il prompt al Player 1
                                if(ssChoiceOwner == 1){
                                    // Default: Attacco scoperto (false). Cambia a true per Difesa scoperta se preferisci.
                                    (void)game->resolvePendingSpecialSummon(/*defense*/false);
                                } else {
                                    // Prompt solo per il Player 1 (in basso)
                                    ssChoiceActive = true;
                                    // Ferma input conflittuali
                                    isDragging = false; isPotentialDrag = false; draggingCardIndex.reset(); potentialDragCardIndex.reset();
                                }
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

        Card tmpcard = Card("", "", 0, 0, sf::Vector2f(0.f, 0.f), sf::Vector2f(0.f, 0.f),
            AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack),
            Type::Monster, Attribute::None, 0, {});

    if(gamestate == GameState::Playing && !returnPopupActive){
            if(game){
                // Gestione pescate: apertura alternata, poi normale turno
                int cur = game->getTurn().getCurrentPlayerIndex();

                // Se attiva l'apertura alternata, metti in coda una pescata a turno tra i due finché non hanno 5 carte
                if(openingAltDrawActive){
                    size_t hand0 = game->getHandOf(0).size();
                    size_t hand1 = game->getHandOf(1).size();
                    if(hand0 >= 5 && hand1 >= 5){
                        openingAltDrawActive = false;
                        openingDrawInFlight = false;
                        openingCurrentPlayer = -1;
                    } else if(drawController.idle() && !openingDrawInFlight){
                        // Se il prossimo ha già 5, passa all'altro
                        int candidate = openingNextPlayer;
                        if(candidate == 0 && hand0 >= 5 && hand1 < 5) candidate = 1;
                        else if(candidate == 1 && hand1 >= 5 && hand0 < 5) candidate = 0;
                        // Accoda una singola pescata per il candidato
                        openingCurrentPlayer = candidate;
                        drawController.queueDraw(1);
                        openingDrawInFlight = true;
                    }
                }

                // Routing della pescata corrente
                bool forceOpening = openingAltDrawActive && openingDrawInFlight && (openingCurrentPlayer == 0 || openingCurrentPlayer == 1);
                Deck &activeDeck = forceOpening ? (openingCurrentPlayer == 0 ? deck : opponentDeck) : ((cur == 0) ? deck : opponentDeck);
                sf::Vector2f activeDeckPos = forceOpening ? (openingCurrentPlayer == 0 ? deckSlotPos : deckSlotPosTop) : ((cur == 0) ? deckSlotPos : deckSlotPosTop);
                std::vector<Card> &drawTargetHand = forceOpening ? game->getHandOf(openingCurrentPlayer) : game->current().getHand();
                float handYForAnim = (forceOpening ? (openingCurrentPlayer == 0 ? y : 30.f) : ((cur == 0) ? y : 30.f));
                
                // Skip reveal when the drawing player is the opponent (P2)
                bool forceSkipReveal = forceOpening ? (openingCurrentPlayer == 1) : (cur == 1);
                drawController.update(deltaTime, mousePressed, forceSkipReveal, activeDeck, drawTargetHand, windowSize, cardSize, spacing, handYForAnim, HAND_MAXSIZE, activeDeckPos,
                    AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack), textureManager, &game->events());
                if(drawController.isShowingDetails()){
                    showDrawnCardDetails = true; tmpcard = drawController.detailCard();
                } else if(showDrawnCardDetails && drawController.idle()){
                    showDrawnCardDetails = false;
                }
                
                // Fine animazione della singola pescata di apertura -> alterna il prossimo giocatore
                if(openingAltDrawActive && openingDrawInFlight && drawController.idle()){
                    openingDrawInFlight = false;
                    // Alterna il prossimo, ma lasceremo la selezione saltare chi ha già 5 al prossimo ciclo
                    openingNextPlayer = 1 - openingCurrentPlayer;
                    openingCurrentPlayer = -1;
                }
                // Aggiorna l'AI del Player 2 solo quando non ci sono animazioni di pescata in corso
                // e quando non è aperto il prompt di risposta per P1 (permette a P1 di attivare nel turno avversario)
                if(drawController.idle() && !responsePromptActive){
                    aiP2.update(deltaTime);
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
                // Navigazione overlay scelta DeckSend con frecce
                if(deckSendChoiceActive){
                    size_t cols = 5; size_t rows = (deckSendCandidates.empty()?0:((deckSendCandidates.size()-1)/cols + 1));
                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)){
                        if(deckSendSelected > 0){ deckSendSelected--; }
                    } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)){
                        if(deckSendSelected + 1 < deckSendCandidates.size()){ deckSendSelected++; }
                    } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)){
                        if(deckSendSelected >= cols){ deckSendSelected -= cols; }
                    } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)){
                        if(deckSendSelected + cols < deckSendCandidates.size()){ deckSendSelected += cols; }
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
        
        // Ottieni la posizione del mouse
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        
        // Aggiorna la posizione della carta durante il dragging
        if (isDragging && draggingCardIndex.has_value() && handPtr && draggingCardIndex.value() < handPtr->size()) {
            sf::Vector2f newCardPos = static_cast<sf::Vector2f>(mousePos) - dragOffset;
            (*handPtr)[draggingCardIndex.value()].setPosition(newCardPos);
        }
        
        ///////////////
        // Rendering //
        ///////////////

        window.clear(sf::Color::Black);

        // Se siamo nello stato StartScreen, disegna il testo sopra lo sfondo con animazione di dissolvenza
        static sf::Clock startScreenClock;
        float startScreenElapsed = startScreenClock.getElapsedTime().asSeconds();
        if(gamestate == GameState::StartScreen) drawStartScreen(window, detailFont, windowSize,
            AppConfig::findTextureInMap(tex, AppConfig::TextureKey::StartScreen), startScreenElapsed);

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
                // Consenti hover anche con prompt di risposta attivo (prompt non-modale)
                bool enableHover = !extraOverlay.isOverlayVisible() && !returnPopupActive && !chooseSummonOrSet && !chooseActivateOrSetST && !deckSendChoiceActive;
                field.draw(window, mousePos, gamestate, enableHover);
            }

            // Disegna i Deck principali (o le animazioni di mescolamento) e gli Extra Deck sopra i relativi slot
            if(field.isAnimationFinished()) {
                if (deck.getSize() > 0) {
                    if (!deck.isShuffleAnimationAdvancedFinished()) {
                        deck.drawShuffleAnimationAdvanced(window);
                    } else {
                        deck.draw(window, mousePos, detailFont, deckSlotPos, slotSize, gamestate);
                    }
                }
                // Deck avversario (in alto)
                if (opponentDeck.getSize() > 0) {
                    if (!opponentDeck.isShuffleAnimationAdvancedFinished()) {
                        opponentDeck.drawShuffleAnimationAdvanced(window);
                    } else {
                        opponentDeck.draw(window, mousePos, detailFont, deckSlotPosTop, slotSize, gamestate);
                    }
                }
                // Extra Deck: disegna sempre lo stack se contiene carte
                if (deck.getExtraSize() > 0) {
                    deck.drawExtra(window, detailFont, extraDeckSlotPos, slotSize, gamestate);
                }
                if (opponentDeck.getExtraSize() > 0) {
                    opponentDeck.drawExtra(window, detailFont, extraDeckSlotPosTop, slotSize, gamestate);
                }
            }

            if (showDrawnCardDetails) {
                sf::Vector2f panelPos{400.f, 150.f};
                sf::Vector2f panelSize{300.f, 200.f};
                showCardDetails(window, tmpcard, detailFont, panelPos, panelSize, scrollOffset);
            }

        // Disegna le carte sul campo (P1)
            for (size_t i=0;i<fieldCards.size();++i) {
                bool isDef=false, isFD=false, showDot=false;
                if(game){
                    // Il lato basso corrisponde sempre al Giocatore 0
                    isDef = game->isDefenseAt(0, i);
                    isFD = game->isFaceDownAt(0, i);
                    if(game->getTurn().getPhase() == GamePhase::Battle){
                        int cur = game->getTurn().getCurrentPlayerIndex();
                        showDot = (cur == 0 && game->hasMonsterAlreadyAttacked(i));
                    }
                }
                bool isSel = selectedAttackerIndex.has_value() && selectedAttackerIndex.value()==i && attackSelectionActive;
                int slotIdx = game ? game->getMonsterSlotIndexAt(0, i) : static_cast<int>(i);
                if(slotIdx < 0) slotIdx = static_cast<int>(i);
                sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, P1, slotIdx);
                RenderUtils::drawMonsterCard(window, fieldCards[i], /*sideTop*/false, slotPos, slotSize, isDef, isFD, isSel, showDot, tex);
            }
            // Disegna anche le Magie/Trappole e Campo (P1)
            RenderUtils::drawSpellTrapRow(window,p1STCards,
                [&](size_t i){ return game && game->isSpellTrapFaceDownAt(0, i); },
                tex
            );
            RenderUtils::drawFieldSpellOpt(window, p1FieldSpellCard);
            // Disegna le carte dell'avversario (P2)
            for (size_t i=0;i<oppFieldCards.size();++i) {
                bool isDef=false, isFD=false, showDot=false;
                if(game){
                    // Il lato alto corrisponde sempre al Giocatore 1
                    isDef = game->isDefenseAt(1, i);
                    isFD = game->isFaceDownAt(1, i);
                    if(game->getTurn().getPhase() == GamePhase::Battle){
                        int cur = game->getTurn().getCurrentPlayerIndex();
                        showDot = (cur == 1 && game->hasMonsterAlreadyAttacked(i));
                    }
                }
                int slotIdx2 = game ? game->getMonsterSlotIndexAt(1, i) : static_cast<int>(i);
                if (slotIdx2 < 0) slotIdx2 = static_cast<int>(i);
                sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, P2, slotIdx2);
                RenderUtils::drawMonsterCard(window, oppFieldCards[i], /*sideTop*/true, slotPos, slotSize, isDef, isFD, /*isSelectedAttacker*/false, showDot, tex);
            }
            // Magie/Trappole e Campo (P2)
            RenderUtils::drawSpellTrapRow(window,p2STCards,
                [&](size_t i){ return game && game->isSpellTrapFaceDownAt(1, i); },
                tex
            );
            RenderUtils::drawFieldSpellOpt(window, p2FieldSpellCard);

            // Disegna la pila Cimitero (semplice: ultima carta)
            {
                std::optional<Card> top;
                if (game && !game->getGraveyard().empty()) top = game->getGraveyard().back();
                RenderUtils::drawTopCardAt(
                    window,
                    top,
                    field.getSlotPosition(Type::Graveyard, P1),
                    slotSize,
                    textureManager,
                    RenderUtils::FieldScale
                );
            }

            // Disegna la pila Cimitero dell'avversario (semplice: ultima carta)
            {
                std::optional<Card> top;
                if (game && !game->getOpponentGraveyard().empty()) top = game->getOpponentGraveyard().back();
                RenderUtils::drawTopCardAt(
                    window,
                    top,
                    field.getSlotPosition(Type::Graveyard, P2),
                    slotSize,
                    textureManager,
                    RenderUtils::FieldScale
                );
            }

            // Disegna le animazioni di scarto in volo (controller)
            discardController.draw(window);
            
            //Disegna le animazioni delle carte
            if(deck.isAnimationFinished()) drawController.draw(window);

            // Disegna le carte in mano (P1 in basso)
            if(handPtr){
            for (size_t i = 0; i < handPtr->size(); ++i) {
                sf::Vector2f originalPos = (*handPtr)[i].getPosition();
                float offset = ssChoiceActive ? 0.f : (*handPtr)[i].getOffset();
                (*handPtr)[i].setPosition(originalPos - sf::Vector2f(0.f, offset));
                (*handPtr)[i].draw(window);
                (*handPtr)[i].setPosition(originalPos);
            }
            }

            // Disegna la mano del P2 (sempre lato alto, carte coperte e capovolte)
            if(game){
                int cur = game->getTurn().getCurrentPlayerIndex();
                const std::vector<Card>& p2HandRef = (cur==0) ? game->opponent().getHand() : game->current().getHand();
                size_t n = p2HandRef.size();
                if(n > 0){
                    float spacingTop = spacing;
                    float totalW = n * cardSize.x + (n - 1) * spacingTop;
                    float startXTop = (windowSize.x - totalW) / 2.f;
                    float yTop = 15.f; // margine alto
                    for(size_t i=0;i<n;++i){
                        Card copy = p2HandRef[i];
                        // mostra sempre il retro per la mano avversaria
                        RenderUtils::applyBackTexture(copy, tex);
                        // ruota 180° e centra nell’area calcolata
                        sf::Vector2f pos(startXTop + i * (cardSize.x + spacingTop), yTop);
                        RenderUtils::rotate180Centered(copy, pos, cardSize);
                        copy.draw(window);
                    }
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
                lpText.setFillColor(sf::Color(255,230,140));
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
                if(responsePromptActive){
                    const std::vector<Card>& source = (responseOwner==0) ? p1STCards : p2STCards;
                    std::vector<std::string> items;
                    items.reserve(responseActivables.size());
                    for(size_t i=0;i<responseActivables.size();++i){
                        size_t zoneIdx = responseActivables[i];
                        std::string name = (zoneIdx < source.size()) ? source[zoneIdx].getName() : std::string("(slot ") + std::to_string(zoneIdx) + ")";
                        items.push_back(std::move(name));
                    }
                    RenderUtils::drawResponsePromptOverlay(window, detailFont, windowSize, items, responseSelected);
                }
                if(deckSendChoiceActive){
                    RenderUtils::drawDeckSendOverlay(
                        window,
                        detailFont,
                        windowSize,
                        deckSendCandidates,
                        deckSendSelected,
                        slotSize,
                        textureManager,
                        0.7f,
                        12.f
                    );
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
            RenderUtils::drawConfirmReturnHomeOverlay(window, detailFont, windowSize, mousePos);
        }

        // Disegno overlay Game Over (sopra tutto)
        if(gameOverActive){
            std::string titleStr = "Game Over";
            if(game && game->isGameOver()){
                auto w = game->getWinnerIndex();
                if(w.has_value()){
                    titleStr = std::string("Vittoria: Player ") + std::to_string(w.value()+1);
                }
            }
            RenderUtils::drawGameOverOverlay(window, detailFont, windowSize, titleStr);
        }

        window.display();
    }
    return 0;
}