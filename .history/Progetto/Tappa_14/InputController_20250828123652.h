#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "../resources/data/GameState.h"
#include "DeckSelectionScreen/DeckSelectionScreen.h"
#include "Card/Card.h"
#include "Config.h"
#include "RenderUtils.h"
#include "GameLogic/Game/Game.h"
#include "GameLogic/GamePhases.h"

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

    // Stato selezioni/overlay da gestire con ESC
    std::optional<size_t>& selectedCardIndex;
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

    // Battle targeting (click destro su mostri avversari)
    bool& attackSelectionActive;
    std::optional<size_t>& selectedAttackerIndex;
    std::vector<Card>& oppFieldCards;
    std::string& battleFeedbackMsg;
    sf::Clock& battleFeedbackClock;

    // Accesso al Game corrente tramite getter (evita dangling pointer)
    std::function<Game*()> getGame;
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
