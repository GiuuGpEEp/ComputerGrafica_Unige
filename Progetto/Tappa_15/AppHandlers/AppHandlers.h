#pragma once

#include <functional>
#include <vector>
#include <optional>
#include <string>
#include "../GameLogic/Game/Game.h"
#include "../Deck/Deck.h"
#include "../Utils/RenderUtils.h"
#include "../Utils/Config.h"
#include <iostream>

class Game;
class Deck;
class Card;

namespace AppHandlers {

struct Context {
    // Callback e riferimenti usati dai gestori
    std::function<void()> syncZones;
    std::function<void()> syncMonsterZoneToField;
    std::function<void()> syncSpellTrapZones;
    std::function<void()> updateHandLayout;
    std::function<void()> openResponsePromptIfAny;
    std::function<void()> rebuildResponseActivables;
    std::function<void()> closeResponsePrompt;

    // Riferimenti UI/stato (puntatori/riferimenti a variabili in main)
    std::function<void()> stopAllInputActions; // es. interrompi il drag

    // Funzionalità UI per il mazzo
    std::function<void()> resetDeckPositions;

    // Accessori/mutatori per lo stato influenzato dai gestori
    std::function<void(bool)> setSelectingTributes;
    std::function<void(int)> setTributesNeeded;
    std::function<void(bool)> setSsChoiceActive;
    std::function<void(bool)> setDeckSendChoiceActive;
    std::function<void(const std::vector<Card>&)> setDeckSendCandidates;
    std::function<void(int)> setDeckSendOwner;
    std::function<void(const std::string&)> pushBattleFeedbackMsg;

    // id delle sottoscrizioni (output)
    int *drawStartSubscriptionId = nullptr;
    int *drawEndSubscriptionId = nullptr;
    int *attackDeclaredSubId = nullptr;
    int *attackResolvedSubId = nullptr;
    int *monsterDestroyedSubId = nullptr;
    int *lifeChangedSubId = nullptr;
    int *directAttackSubId = nullptr;
};

// Attacca i gestori che si sottoscrivono agli eventi di Game. Il Context deve
// vivere più a lungo delle sottoscrizioni.
void attachGameHandlers(Game &game, const Context &ctx);

// Stacca i gestori attaccati con attachGameHandlers. Sicuro chiamarlo più volte.
void detachGameHandlers(Game &game, const Context &ctx);

} // namespace AppHandlers
