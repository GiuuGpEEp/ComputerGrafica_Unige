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
#include "Field/Field.h"

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
