#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "../../resources/data/GameState.h"
#include "../DeckSelectionScreen/DeckSelectionScreen.h"
#include "../ExtraDeckOverlay/ExtraDeckOverlay.h"
#include "../GraveyardOverlay/GraveyardOverlay.h"
#include "../Card/Card.h"
#include "../Utils/Config.h"
#include "../Utils/RenderUtils.h"
#include "../GameLogic/Game/Game.h"
#include "../GameLogic/GamePhases.h"
#include "../Utils/SFMLCompat.h"

namespace Input {

struct Context {
    // Elementi base finestra/stato
    sf::RenderWindow& window;
    sf::Vector2u& windowSize;
    GameState& gamestate;

    // Popup/overlay globali
    bool& returnPopupActive;
    bool& gameOverActive;

    // Callback di reset partita (main possiede la logica)
    std::function<void()> resetMatch;

    // Predicato per sapere se l'input è bloccato (overlay/prompt attivi)
    std::function<bool()> inputBlocked;

    // Selezione mazzo
    DeckSelectionScreen& deckSelectionScreen;

    // Overlay: Extra/Graveyard gestione hold/click
    ExtraDeckOverlay& extraOverlay;
    GraveyardOverlay& graveyardOverlay;
    sf::Vector2f extraDeckSlotPos;   // posizione slot Extra (P1)
    sf::Vector2f graveyardSlotPos;   // posizione slot Cimitero (P1)
    sf::Vector2f graveyardSlotPosTop; // posizione slot Cimitero (P2)
    std::function<std::vector<Card>()> getExtraCards;       // sorgente carte Extra
    std::function<std::vector<Card>()> getGraveyardCards;   // sorgente Cimitero corrente

    // Overlay: scelta invio dal Deck
    bool& deckSendChoiceActive;
    size_t& deckSendSelected;
    std::vector<Card>& deckSendCandidates;
    sf::Vector2f slotSize;
    int& deckSendOwner;
    std::function<void(size_t)> confirmDeckSend;

    // Prompt di risposta
    bool& responsePromptActive;
    size_t& responseSelected;
    std::function<size_t()> responseOptionsCount;
    std::function<void()> closeResponsePrompt;
    std::function<void(size_t)> confirmResponsePrompt;
    std::function<void()> openResponsePromptIfAny; // facoltativa: apertura prompt dopo avanzamento fase

    // Stato selezioni/overlay da gestire con ESC
    std::optional<size_t>& selectedCardIndex;
    bool& selectedCardIsOnField;
    bool& selectedCardIsSpellTrap; // true if the selected card is from Spell/Trap zone
    int& selectedCardSTOwner; // owner absolute (0=P1 bottom,1=P2 top) when selectedCardIsSpellTrap==true
    float& scrollOffset;
    std::function<void()> extraOnEscape; // chiude overlay extra deck
    std::function<void()> graveOnEscape; // chiude overlay cimitero
    bool& selectingTributes;
    std::vector<size_t>& selectedTributes;
    int& tributesNeeded;
    bool& chooseSummonOrSet;
    bool& chooseActivateOrSetST;
    std::optional<size_t>& pendingHandIndexForPlay;
    bool& pendingIsSpellTrap;
    std::function<void(size_t)> restoreCardAtHandIndex; // ripristina carta nella mano
    std::function<void()> syncMonsterZoneToField;       // resync grafico
    // Callback fornita dal main per annullare un drag attivo e riportare la carta in mano
    std::function<void()> cancelActiveDrag;

    // Battle targeting (click destro su mostri avversari)
    bool& attackSelectionActive;
    std::optional<size_t>& selectedAttackerIndex;
    std::vector<Card>& oppFieldCards;
    std::string& battleFeedbackMsg;
    sf::Clock& battleFeedbackClock;

    // Riferimento alla lista ST del player 1 (necessario per hit-test con X)
    std::vector<Card>& p1STCards;

    // Permette al controller di richiedere il sync delle ST dopo attivazioni
    std::function<void()> syncSpellTrapZones;

    // Callback per aggiornare il layout della mano (definita in main)
    std::function<void()> updateHandLayout;

    // Funzione ausiliaria fornita da main per mappare la posizione del mouse su uno slot mostro
    std::function<std::optional<int>(sf::Vector2f)> findMonsterSlotIndexUnderMouse;

    // Accesso al Game corrente tramite getter (evita dangling pointer)
    std::function<Game*()> getGame;

    // Risorsa necessaria per creare carte di debug (per tasto O)
    sf::Texture* cardBackTexture;
};

class Controller {
public:
    explicit Controller(Context state) : state(state) {}

    // Ritorna true se l'evento è stato gestito e non va propagato oltre
    bool handleEvent(const sf::Event& e);

private:
    Context state;
};

} // namespace Input
