#pragma once

#include <functional>
#include <vector>
#include <optional>
#include <string>
#include "GameLogic/Game/Game.h"
#include "Deck/Deck.h"
#include "../Utilis/RenderUtils.h"
#include "Config.h"
#include <iostream>

class Game;
class Deck;
class Card;

namespace AppHandlers {

struct Context {
    // Callbacks and references used by handlers
    std::function<void()> syncZones;
    std::function<void()> syncMonsterZoneToField;
    std::function<void()> syncSpellTrapZones;
    std::function<void()> updateHandLayout;
    std::function<void()> openResponsePromptIfAny;
    std::function<void()> rebuildResponseActivables;
    std::function<void()> closeResponsePrompt;

    // UI/state references (pointers/references to variables in main)
    std::function<void()> stopAllInputActions; // e.g. stop dragging

    // Deck UI helpers
    std::function<void()> resetDeckPositions;

    // Accessors/mutators for state that handlers affect
    std::function<void(bool)> setSelectingTributes;
    std::function<void(int)> setTributesNeeded;
    std::function<void(bool)> setSsChoiceActive;
    std::function<void(bool)> setDeckSendChoiceActive;
    std::function<void(const std::vector<Card>&)> setDeckSendCandidates;
    std::function<void(int)> setDeckSendOwner;
    std::function<void(const std::string&)> pushBattleFeedbackMsg;

    // subscription ids outputs
    int *drawStartSubscriptionId = nullptr;
    int *drawEndSubscriptionId = nullptr;
    int *attackDeclaredSubId = nullptr;
    int *attackResolvedSubId = nullptr;
    int *monsterDestroyedSubId = nullptr;
    int *lifeChangedSubId = nullptr;
    int *directAttackSubId = nullptr;
};

// Attach handlers that subscribe to Game events. The Context must outlive the subscriptions.
void attachGameHandlers(Game &game, const Context &ctx);

// Detach handlers that were attached with attachGameHandlers. Safe to call multiple times.
void detachGameHandlers(Game &game, const Context &ctx);

} // namespace AppHandlers
