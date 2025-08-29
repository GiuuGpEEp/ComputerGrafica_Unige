#include "Deck/ShuffleAnimation.h"
#include "Utils/auxFunc.h"
#include "HomePage/HomePage.h"
#include "FieldLoadingAnimation/FieldLoadingAnimation.h"
#include "GameLogic/DrawController/DrawController.h"
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <fstream>

// Controller AI
#include "GameLogic/AI/AIController.h"

#include "InputController.h"
#include "GameWiring.h"

#define P1 1 
#define P2 2
#define DECK_SIZE 30 // numero di carte nel deck
#define HAND_MAXSIZE 7 
// CARD_MAXOFFSET mantenuto per compatibilità con i commenti; non usato direttamente.
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

    // Stato Fase di Battaglia (v1)
    bool attackSelectionActive = false;
    std::optional<size_t> selectedAttackerIndex;
    std::string battleFeedbackMsg;
    sf::Clock battleFeedbackClock;
    const float BATTLE_FEEDBACK_DURATION = AppConfig::Timing::BattleFeedbackSec; 

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
    float deckScaleFactor = AppConfig::Ui::DeckScaleFactor; 
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
    float spacing = AppConfig::Ui::HandSpacingPx;
    float totalHandWidth = initialCard * cardSize.x + (initialCard - 1) * spacing;
    float startX = (windowSize.x - totalHandWidth) / 2.f;
    
    //Ripristino la posizione originale delle carte
    float y = windowSize.y - cardSize.y - AppConfig::Ui::HandBottomMarginPx; 

    float scrollOffset = 0.f; //Offset per lo scroll del testo dei dettagli della carta

    // Variabili per il dragging
    bool isDragging = false;
    bool isPotentialDrag = false; // Indica se il mouse è premuto su una carta ma non ancora in dragging
    std::optional<size_t> draggingCardIndex;
    std::optional<size_t> potentialDragCardIndex;
    sf::Vector2f dragOffset; // Offset tra mouse e posizione carta
    sf::Vector2f initialMousePos; // Posizione iniziale del mouse al click
    const float dragThreshold = AppConfig::Ui::DragThresholdPx; // Soglia in pixel per attivare il dragging
    
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
    auto syncMonsterZoneToField = [&](){ RenderUtils::syncMonsterZoneToField(game.get(), field, fieldCards, oppFieldCards, textureManager, slotSize, selectedCardIsOnField, selectedCardIndex); };

    // Sincronizza M/T e Field Spell con lo stato logico
    auto syncSpellTrapZones = [&](){ RenderUtils::syncSpellTrapZones(game.get(), field, p1STCards, p2STCards, p1FieldSpellCard, p2FieldSpellCard, textureManager, slotSize); };
    // Wrapper per sincronizzare entrambe le viste (Mostri + M/T/Field)
    auto syncZones = [&](){ RenderUtils::syncZones(game.get(), field, fieldCards, oppFieldCards, p1STCards, p2STCards, p1FieldSpellCard, p2FieldSpellCard, textureManager, slotSize, selectedCardIsOnField, selectedCardIndex); };
    // Context for AppHandlers subscriptions (hoisted so reset can detach safely)
    AppHandlers::Context appHandlersCtx;
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
        
        // Detach handlers before destroying the Game to avoid dangling callbacks
        if(game){
            try {
                AppHandlers::detachGameHandlers(*game, appHandlersCtx);
            } catch(...) { /* swallow any issue during detach */ }
        }
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
    // Nota: qui richiamiamo direttamente updateHandPositions
    // poiche' updateHandLayout e' definita di seguito a questa lambda.
    updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
    };
    // Wrapper per aggiornare rapidamente il layout della mano
    auto updateHandLayout = [&](){ if(handPtr){ updateHandPositions(*handPtr, windowSize, cardSize, spacing, y, HAND_MAXSIZE); } };
    
    // Avviso se si tenta di giocare senza aver selezionato un deck
    bool showNoDeckWarning = false;
    sf::Clock noDeckWarningClock;
    const float NO_DECK_WARNING_DURATION = AppConfig::Timing::NoDeckWarningSec; // secondi
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

    // Helper per sapere se l'input utente va bloccato (modale attivo, popup, ecc.)
    auto inputBlocked = [&](){
        return selectingTributes || ssChoiceActive || responsePromptActive || deckSendChoiceActive || returnPopupActive || gameOverActive;
    };

    // Setup dello stato --> preparo in una struct le variabili da passare poi al controller
    Input::Context state{
        /*window*/window,
        /*windowSize*/windowSize,
        /*gamestate*/gamestate,
        /*returnPopupActive*/returnPopupActive,
        /*gameOverActive*/gameOverActive,
        /*resetMatch*/resetMatch,
        /*inputBlocked*/[&](){ return inputBlocked(); },
        /*deckSelectionScreen*/deckSelectionScreen,
        /*extraOverlay*/extraOverlay,
        /*graveyardOverlay*/graveyardOverlay,
        /*extraDeckSlotPos*/extraDeckSlotPos,
        /*graveyardSlotPos*/field.getSlotPosition(Type::Graveyard, P1),
        /*getExtraCards*/[&](){ return deck.getExtraCards(); },
        /*getGraveyardCards*/[&](){ return game ? game->getGraveyard() : std::vector<Card>{}; },
        /*deckSendChoiceActive*/deckSendChoiceActive,
        /*deckSendSelected*/deckSendSelected,
        /*deckSendCandidates*/deckSendCandidates,
        /*slotSize*/slotSize,
        /*deckSendOwner*/deckSendOwner,
        /*confirmDeckSend*/[&](size_t idx){
            if(game){
                if(game->resolvePendingSendFromDeck(idx)){
                    deckSendChoiceActive = false; deckSendCandidates.clear(); deckSendSelected = 0; deckSendOwner = -1;
                    battleFeedbackMsg = "Carta inviata al Cimitero"; battleFeedbackClock.restart();
                } else {
                    battleFeedbackMsg = "Selezione non valida"; battleFeedbackClock.restart();
                }
            }
        },
        /*responsePromptActive*/responsePromptActive,
        /*responseSelected*/responseSelected,
        /*responseOptionsCount*/[&](){ return responseActivables.size(); },
        /*closeResponsePrompt*/[&](){ closeResponsePrompt(); },
    /*confirmResponsePrompt*/[&](size_t sel){
            if(game && sel < responseActivables.size()){
                size_t zoneIdx = responseActivables[sel];
                bool ok = game->activateSetSpellTrapFor(responseOwner, zoneIdx);
                if(ok){
                    battleFeedbackMsg = "Carta attivata"; battleFeedbackClock.restart();
                    syncSpellTrapZones();
                } else {
                    battleFeedbackMsg = "Impossibile attivarla ora"; battleFeedbackClock.restart();
                }
                closeResponsePrompt();
            }
    },
    /*openResponsePromptIfAny*/[&](){ openResponsePromptIfAny(); },
    /*selectedCardIndex*/selectedCardIndex,
    /*selectedCardIsOnField*/selectedCardIsOnField,
    /*scrollOffset*/scrollOffset,
    /*extraOnEscape*/[&](){ extraOverlay.handleEscape(); },
    /*graveOnEscape*/[&](){ graveyardOverlay.handleEscape(); },
    /*selectingTributes*/selectingTributes,
    /*selectedTributes*/selectedTributes,
    /*tributesNeeded*/tributesNeeded,
    /*chooseSummonOrSet*/chooseSummonOrSet,
    /*chooseActivateOrSetST*/chooseActivateOrSetST,
    /*pendingHandIndexForPlay*/pendingHandIndexForPlay,
    /*pendingIsSpellTrap*/pendingIsSpellTrap,
    /*restoreCardAtHandIndex*/[&](size_t idx){ restoreCardAtHandIndex(idx); },
    /*syncMonsterZoneToField*/[&](){ syncMonsterZoneToField(); },
        /*attackSelectionActive*/attackSelectionActive,
        /*selectedAttackerIndex*/selectedAttackerIndex,
        /*oppFieldCards*/oppFieldCards,
        /*battleFeedbackMsg*/battleFeedbackMsg,
        /*battleFeedbackClock*/battleFeedbackClock,
    /*p1STCards*/p1STCards,
    /*syncSpellTrapZones*/[&](){ syncSpellTrapZones(); },
    /*updateHandLayout*/[&](){ updateHandLayout(); },
    /*findMonsterSlotIndexUnderMouse*/[&](sf::Vector2f mp){ return findMonsterSlotIndexUnderMouse(mp, field, slotSize); },
    /*getGame*/[&](){ return game.get(); },
        /*cardBackTexture*/ &AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack)
    };
    Input::Controller inputController(state);

    while (window.isOpen()) {

        ///////////////////////////
        // Gestione Degli Eventi //
        ///////////////////////////

        while (const std::optional event = window.pollEvent()){
            // Delega blocchi comuni all'InputController; se gestito, continua
            if(inputController.handleEvent(*event)) continue;

            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                // Tasti debug e attacco diretto ora sono gestiti dal Controller
            }
            
            if (const auto *mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Mouse::Button but = mouseButton->button;
                // Nota: molte vie del mouse sono già gestite da InputController; qui restano i casi specifici del main

                if(but == M_MOUSE_LEFT) {
                    mousePressed = true;
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    initialMousePos = static_cast<sf::Vector2f>(mousePos);

                    // Hold/click overlay spostati nel Controller

                    if(!inputBlocked() && gamestate == GameState::HomeScreen && homeScreen.getGiocaBounds().contains(initialMousePos)) {
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
                            GameWiring::createGameAndAttachResources(selectedDeckName, deck, opponentDeck, resourceManager, game, drawController, aiP2, handPtr, p1STCards, p2STCards, p1FieldSpellCard, p2FieldSpellCard);
                            // Le sottoscrizioni agli eventi verranno impostate dopo l'avvio del gioco
                            gamestate = GameState::FieldLoading; // stato intermedio con animazione
                        }
                    } 

                    if(!inputBlocked() && gamestate == GameState::HomeScreen && homeScreen.getSelectionBounds().contains(initialMousePos)) {
                        std::cout << "Cliccato su Selezione Mazzo!" << std::endl;
                        gamestate = GameState::DeckSelection;
                        deckSelectionScreen.startFadeIn(AppConfig::Timing::FadeInFastSec);
                    }

                    if(!inputBlocked() && gamestate == GameState::DeckSelection){
                        if(deckSelectionScreen.getDeckBlueEyesBounds().contains(initialMousePos)) {
                            std::cout << "Selezionato mazzo Blue Eyes!" << std::endl;
                            selectedDeckName = "Blu-Eyes";
                            deckSelected = true;
                            deckSelectionScreen.setSelectedDeck(0);
                            homeScreen.setDeckName(selectedDeckName);
                            // Avvia fade out dopo selezione
                            if(!deckSelectionScreen.isFading()) deckSelectionScreen.startFadeOut(AppConfig::Timing::FadeOutFastSec);
                        } else if(deckSelectionScreen.getDeckDarkMagicianBounds().contains(initialMousePos)) {
                            std::cout << "Selezionato mazzo Dark Magician!" << std::endl;
                            selectedDeckName = "Black-Magician";
                            deckSelected = true;
                            deckSelectionScreen.setSelectedDeck(1);
                            homeScreen.setDeckName(selectedDeckName);
                            if(!deckSelectionScreen.isFading()) deckSelectionScreen.startFadeOut(AppConfig::Timing::FadeOutFastSec);
                        } else if(deckSelectionScreen.getDeckRedEyesBounds().contains(initialMousePos)) {
                            std::cout << "Selezionato mazzo Red Eyes!" << std::endl;
                            selectedDeckName = "Red-Eyes";
                            deckSelected = true;
                            deckSelectionScreen.setSelectedDeck(2);
                            homeScreen.setDeckName(selectedDeckName);
                            if(!deckSelectionScreen.isFading()) deckSelectionScreen.startFadeOut(AppConfig::Timing::FadeOutFastSec);
                        }
                    }
                }
            }

            if(mousePressed && !isDragging) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                bool cardClicked = false;
                // Controlla prima le carte in mano per il drag
                if(handPtr && !inputBlocked()){
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
                                auto it = std::find(selectedTributes.begin(), selectedTributes.end(), i);
                                if(it == selectedTributes.end()){ // Controllo che il mostro cliccato non sia stato già selezionato
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
                                            updateHandLayout();
                                        }
                                        std::cout << "[Tribute] Summon completata" << std::endl;
                                    } else {
                                        std::cout << "[Tribute] Completamento summon fallito" << std::endl;
                                    }
                                }
                            }
                            // Battaglia: seleziona l'attaccante con il click (disabilitato durante selezione tributi)
                            if(game && !inputBlocked() && game->getTurn().getPhase() == GamePhase::Battle){
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
                if(but == M_MOUSE_LEFT) {
                    // Rilascio overlay gestito dal Controller

                    //funzione per far tornare la carta in mano in caso di posizione errata
                    auto cardBackToHand = [&](){
                        Card tempCard = (*handPtr)[draggingCardIndex.value()];
                        handPtr->erase(handPtr->begin() + draggingCardIndex.value());
                        sf::Vector2f originalPos = setHandPos(*handPtr, tempCard, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                        handPtr->insert(handPtr->begin() + draggingCardIndex.value(), tempCard);
                        (*handPtr)[draggingCardIndex.value()].setPosition(originalPos);
                        updateHandLayout();
                    };

                    if (!inputBlocked() && isDragging && draggingCardIndex.has_value()) {
                        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                        bool hasFreeSlotUnderMouse = false;
                        if(handPtr){
                            hasFreeSlotUnderMouse = findSlotPosition(mousePos, (*handPtr)[draggingCardIndex.value()], field, slotSize).has_value();
                        }
                        // Controlla se la carta in drag è un mostro che richiede tributi
                        bool canOpenSummonChoiceForTribute = false;
                        if(game && handPtr && !inputBlocked()){
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
                            if(game && !inputBlocked()){
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
             
            // Scroll mouse
            if (const auto* mouseScroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                // Permetti lo scroll sia se una carta è selezionata, sia se si sta mostrando la carta pescata
                if (selectedCardIndex.has_value() || showDrawnCardDetails || extraOverlay.detailsActive() || graveyardOverlay.detailsActive()) {
                    scrollOffset -= mouseScroll->delta * AppConfig::Ui::DetailsScrollStepPx;
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
            GameWiring::createGameAndAttachResources(selectedDeckName, deck, opponentDeck, resourceManager, game, drawController, aiP2, handPtr, p1STCards, p2STCards, p1FieldSpellCard, p2FieldSpellCard);
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
        if (isPotentialDrag && potentialDragCardIndex.has_value() && !inputBlocked()) {
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
                            syncZones();
                            // populate the hoisted context and attach using helper
                            GameWiring::setupAppHandlers(
                                game,
                                appHandlersCtx,
                                /*syncZones*/[&](){ syncZones(); },
                                /*syncMonsterZoneToField*/[&](){ syncMonsterZoneToField(); },
                                /*syncSpellTrapZones*/[&](){ syncSpellTrapZones(); },
                                /*updateHandLayout*/[&](){ if(handPtr) updateHandLayout(); },
                                /*openResponsePromptIfAny*/[&](){ openResponsePromptIfAny(); },
                                /*rebuildResponseActivables*/[&](){ rebuildResponseActivables(); },
                                /*closeResponsePrompt*/[&](){ closeResponsePrompt(); },
                                /*stopAllInputActions*/[&](){ isDragging = false; isPotentialDrag = false; draggingCardIndex.reset(); potentialDragCardIndex.reset(); },
                                /*resetDeckPositions*/[&](){ 
                                    deck.resetDeckCardPositions(deckSlotPos, deckCardSize, slotSize, AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack)); 
                                    opponentDeck.resetDeckCardPositions(deckSlotPosTop, deckCardSize, slotSize, AppConfig::findTextureInMap(tex, AppConfig::TextureKey::CardBack)); 
                                },
                                /*setSelectingTributes*/[&](bool v){ selectingTributes = v; },
                                /*setTributesNeeded*/[&](int v){ tributesNeeded = v; },
                                /*setSsChoiceActive*/[&](bool v){ ssChoiceActive = v; },
                                /*setDeckSendChoiceActive*/[&](bool v){ deckSendChoiceActive = v; },
                                /*setDeckSendCandidates*/[&](const std::vector<Card>& c){ deckSendCandidates = c; },
                                /*setDeckSendOwner*/[&](int o){ deckSendOwner = o; },
                                /*pushBattleFeedbackMsg*/[&](const std::string &s){ battleFeedbackMsg = s; battleFeedbackClock.restart(); },
                                &drawStartSubscriptionId,
                                &drawEndSubscriptionId,
                                &attackDeclaredSubId,
                                &attackResolvedSubId,
                                &monsterDestroyedSubId,
                                &lifeChangedSubId,
                                &directAttackSubId,
                                deck,
                                opponentDeck,
                                deckSlotPos,
                                deckSlotPosTop,
                                deckCardSize,
                                slotSize,
                                drawController,
                                discardController,
                                field,
                                handPtr,
                                tex
                            );

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
                float handYForAnim = (forceOpening ? (openingCurrentPlayer == 0 ? y : AppConfig::Ui::OpponentHandTopMarginPx)
                                                   : ((cur == 0) ? y : AppConfig::Ui::OpponentHandTopMarginPx));
                
                // Salta la rivelazione quando il giocatore che pesca è l'avversario (P2)
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
                if(drawController.idle() && !inputBlocked()){
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
                        updateHandLayout();
                        game->onDiscardAnimationFinished();
                        std::cout << "[AutoTurn] Fine turno dopo animazione scarto." << std::endl;
                    }
                }
                // Gestione input per overlay scelta Evocazione Speciale (A=Attacco, D=Difesa)
                if(ssChoiceActive){
                    if(sf::Keyboard::isKeyPressed(M_KEY_A)){
                        if(game->resolvePendingSpecialSummon(/*defense*/false)){
                            ssChoiceActive = false; ssChoiceOwner = -1;
                        }
                    } else if(sf::Keyboard::isKeyPressed(M_KEY_D)){
                        if(game->resolvePendingSpecialSummon(/*defense*/true)){
                            ssChoiceActive = false; ssChoiceOwner = -1;
                        }
                    }
                }
                // Scelta post-drop per Spell/Trap: A=Attiva, S=Setta
                if(pendingHandIndexForPlay.has_value() && pendingIsSpellTrap && chooseActivateOrSetST){
                    if(sf::Keyboard::isKeyPressed(M_KEY_A)){
                        if(game->activateSpellFromHand(pendingHandIndexForPlay.value())){
                            updateHandLayout();
                        } else {
                            restoreCardAtHandIndex(pendingHandIndexForPlay.value());
                            battleFeedbackMsg = "Attivazione non riuscita"; battleFeedbackClock.restart();
                        }
                        chooseActivateOrSetST = false; pendingHandIndexForPlay.reset(); pendingIsSpellTrap = false;
                    } else if(sf::Keyboard::isKeyPressed(M_KEY_S)){
                        size_t idx = pendingHandIndexForPlay.value();
                        bool ok = false;
                        if((*handPtr)[idx].getType() == Type::FieldSpell){ ok = game->setFieldSpell(idx); }
                        else { ok = game->setSpellOrTrap(idx); }
                        if(ok){
                            updateHandLayout();
                        } else {
                            restoreCardAtHandIndex(idx);
                            battleFeedbackMsg = "Set non riuscito"; battleFeedbackClock.restart();
                        }
                        chooseActivateOrSetST = false; pendingHandIndexForPlay.reset(); pendingIsSpellTrap = false;
                    }
                }
                // Navigazione overlay DeckSend spostata nel Controller
            }
        }
    

        // Gestione dell'hold dell'Extra Deck (attiva la preview dopo una soglia se non si stanno trascinando carte)
        if(!returnPopupActive){
            extraOverlay.update(deltaTime, isDragging, gamestate);
            graveyardOverlay.update(deltaTime, isDragging, gamestate);
        }

        // Gestione del sollevamento delle carte in mano
        float targetCardOffset = 0.f;
        float offsetSpeed = AppConfig::Ui::LiftSpeedPxPerSec; 
        if(!returnPopupActive){
            if(handPtr){
            for(size_t i = 0; i<handPtr->size(); ++i){
                float currentOffset = (*handPtr)[i].getOffset();
                if(selectedCardIndex.has_value() && !selectedCardIsOnField && selectedCardIndex.value() == i) {
                    targetCardOffset = AppConfig::Ui::CardLiftPx;
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
        if(gamestate == GameState::StartScreen) 
            drawStartScreen(window, detailFont, windowSize, AppConfig::findTextureInMap(tex, AppConfig::TextureKey::StartScreen), startScreenElapsed);

        if(gamestate == GameState::HomeScreen){
            
            homeScreen.draw(window);
            if(showNoDeckWarning){
                float elapsed = noDeckWarningClock.getElapsedTime().asSeconds();
                RenderUtils::drawNoDeckWarning(window, noDeckWarningText, windowSize, elapsed, NO_DECK_WARNING_DURATION, AppConfig::Timing::NoDeckWarningFadeSec);
                if(elapsed >= NO_DECK_WARNING_DURATION) showNoDeckWarning = false;
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
                // Se i vari prompt sono chiusi allora l'hover è abilitato
                bool enableHover = !extraOverlay.isOverlayVisible() && !inputBlocked() && !chooseSummonOrSet && !chooseActivateOrSetST;
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
                RenderUtils::drawDetailsPanel(window, tmpcard, detailFont, windowSize, scrollOffset);
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
                bool isSelected = selectedAttackerIndex.has_value() && selectedAttackerIndex.value()==i && attackSelectionActive;
                int slotIdx = game ? game->getMonsterSlotIndexAt(0, i) : static_cast<int>(i);
                if(slotIdx < 0) slotIdx = static_cast<int>(i);
                sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, P1, slotIdx);
                RenderUtils::drawMonsterCard(window, fieldCards[i], /*isOpponentSide*/false, slotPos, slotSize, isDef, isFD, isSelected, showDot, tex);
            }
            // Disegna anche le Magie/Trappole e Campo (P1)
            RenderUtils::drawSpellTrapRow(window,p1STCards,
                [&](size_t i){ return game && game->isSpellTrapFaceDownAt(0, i); }, // Funzione che controlla se la Magia/Trappola è coperta
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
                RenderUtils::drawMonsterCard(window, oppFieldCards[i], /*isOpponentSide*/true, slotPos, slotSize, isDef, isFD, /*isSelectedAttacker*/false, showDot, tex);
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
            if(handPtr){ RenderUtils::drawPlayerHand(window, *handPtr, ssChoiceActive); }

            // Disegna la mano del P2 (sempre lato alto, carte coperte e capovolte)
            if(game){
                int cur = game->getTurn().getCurrentPlayerIndex();
                const std::vector<Card>& p2HandRef = (cur==0) ? game->opponent().getHand() : game->current().getHand();
                RenderUtils::drawOpponentHand(window, p2HandRef, windowSize, cardSize, spacing, tex);
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
                    RenderUtils::drawDetailsPanel(window, *selectedCard, detailFont, windowSize, scrollOffset);
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
                    RenderUtils::drawOverlayDetails(window, *c, detailFont, windowSize, scrollOffset);
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
                RenderUtils::drawTopLeftHud(window, detailFont, hud);
                // LP HUD in alto a destra
                int lpCur = game->current().getLifePoints();
                int lpOpp = game->opponent().getLifePoints();
                RenderUtils::drawLpHud(window, detailFont, windowSize, lpCur, lpOpp);
                if(selectingTributes){
                    RenderUtils::drawTributePrompt(window, detailFont);
                }
                if(over>0){
                    RenderUtils::drawHandOverflowWarn(window, detailFont);
                }
                if(ssChoiceActive){
                    RenderUtils::drawSSChoicePrompt(window, detailFont);
                }
                if(chooseActivateOrSetST){
                    RenderUtils::drawSTChoicePrompt(window, detailFont);
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
                        AppConfig::Ui::DeckSendScale,
                        AppConfig::Ui::DeckSendPad
                    );
                }
                // Suggerimenti e feedback per la fase Battle
                if(game->getTurn().getPhase() == GamePhase::Battle){
                    RenderUtils::drawBattleHint(window, detailFont);
                }
                if(!battleFeedbackMsg.empty()){
                    float elapsed = battleFeedbackClock.getElapsedTime().asSeconds();
                    if(elapsed < BATTLE_FEEDBACK_DURATION){
                        RenderUtils::drawCenterFeedback(window, detailFont, windowSize, battleFeedbackMsg, AppConfig::Ui::HudSize, AppConfig::Ui::CenterFeedbackY);
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
