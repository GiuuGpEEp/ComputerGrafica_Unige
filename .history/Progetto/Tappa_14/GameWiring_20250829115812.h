#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <functional>
#include "GameLogic/Game/Game.h"
#include "Deck/Deck.h"
#include "DrawAnimation/DrawAnimation.h"
#include "GameLogic/AI/AIController.h"
#include "DrawAnimation/DrawAnimation.h"
#include "ResourceManager/ResourceManager.h"
#include "TextureManager/TextureManager.h"
#include "Config.h"
#include "AppHandlers.h"
#include "Field/Field.h"
#include "GameLogic/DiscardController/DiscardController.h

#include "GameLogic/Game/Effects/cards/WhiteStoneLegendEffect.h"
#include "GameLogic/Game/Effects/cards/WhiteStoneAncientsEffect.h"
#include "GameLogic/Game/Effects/cards/ReliquarioDelDragoEffect.h"
#include "GameLogic/Game/Effects/cards/PiantoDiArgentoEffect.h"


"

// Forward declarations for classes used by main
class DrawController;
class DiscardController;

// Register a small set of default effects/activations used by the demo games.
void registerDefaultEffects(Game &game);

// Create a new Game instance and attach controllers/resources. Centralizes the
// repetitive initialization logic used in multiple places in main.
void createGameAndAttachResources(
    const std::string &selectedDeckName,
    Deck &deck,
    Deck &opponentDeck,
    ResourceManager &resourceManager,
    std::unique_ptr<Game> &game,
    DrawController &drawController,
    AIController &aiP2,
    std::vector<Card>* &handPtr,
    std::vector<Card> &p1STCards,
    std::vector<Card> &p2STCards,
    std::optional<Card> &p1FieldSpellCard,
    std::optional<Card> &p2FieldSpellCard
);

// Helper to populate AppHandlers::Context and attach handlers for a started game.
void setupAppHandlers(
    std::unique_ptr<Game> &game,
    AppHandlers::Context &appHandlersCtx,
    std::function<void()> syncZones,
    std::function<void()> syncMonsterZoneToField,
    std::function<void()> syncSpellTrapZones,
    std::function<void()> updateHandLayout,
    std::function<void()> openResponsePromptIfAny,
    std::function<void()> rebuildResponseActivables,
    std::function<void()> closeResponsePrompt,
    std::function<void()> stopAllInputActions,
    std::function<void()> resetDeckPositions,
    std::function<void(bool)> setSelectingTributes,
    std::function<void(int)> setTributesNeeded,
    std::function<void(bool)> setSsChoiceActive,
    std::function<void(bool)> setDeckSendChoiceActive,
    std::function<void(const std::vector<Card>&)> setDeckSendCandidates,
    std::function<void(int)> setDeckSendOwner,
    std::function<void(const std::string&)> pushBattleFeedbackMsg,
    int *drawStartSubscriptionId,
    int *drawEndSubscriptionId,
    int *attackDeclaredSubId,
    int *attackResolvedSubId,
    int *monsterDestroyedSubId,
    int *lifeChangedSubId,
    int *directAttackSubId,
    Deck &deck,
    Deck &opponentDeck,
    const sf::Vector2f &deckSlotPos,
    const sf::Vector2f &deckSlotPosTop,
    const sf::Vector2f &deckCardSize,
    const sf::Vector2f &slotSize,
    DrawController &drawController,
    DiscardController &discardController,
    Field &field,
    std::vector<Card>* &handPtr,
    const decltype(AppConfig::buildTextureMap(std::declval<TextureManager&>())) &tex
);
