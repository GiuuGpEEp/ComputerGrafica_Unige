#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <functional>
#include "../GameLogic/Game/Game.h"
#include "../Deck/Deck.h"
#include "../DrawAnimation/DrawAnimation.h"
#include "../GameLogic/AI/AIController.h"
#include "../ResourceManager/ResourceManager.h"
#include "../TextureManager/TextureManager.h"
#include "../Utils/Config.h"
#include "../AppHandlers/AppHandlers.h"
#include "../Field/Field.h"
#include "../GameLogic/DiscardController/DiscardController.h"

#include "../GameLogic/Game/Effects/cards/BluEyes/WhiteStoneLegends/WhiteStoneLegendEffect.h"
#include "../GameLogic/Game/Effects/cards/BluEyes/WhiteStoneAncients/WhiteStoneAncientsEffect.h"
#include "../GameLogic/Game/Effects/cards/BluEyes/ReliquarioDelDrago/ReliquarioDelDragoEffect.h"
#include "../GameLogic/Game/Effects/cards/BluEyes/PiantoDiArgento/PiantoDiArgentoEffect.h"
#include "../GameLogic/Game/Effects/cards/BluEyes/LaMelodiaDelDragoCheSiRisveglia/LaMelodiaDelDragoCheSiRisvegliaEffect.h"
#include "../GameLogic/Game/Effects/cards/BluEyes/RichiamoDelPosseduto/RichiamoDelPossedutoEffect.h"
#include "../GameLogic/Game/Effects/cards/BluEyes/RivaliPredestinati/RivaliPredestinatiEffect.h"
// Saggio con Occhi di Blu (on-summon)
#include "../GameLogic/Game/Effects/cards/BluEyes/SageWithEyesOfBlue/SageWithEyesOfBlueEffect.h"
// Fanciulla con gli Occhi di Blu (on target by attack)
#include "../GameLogic/Game/Effects/cards/BluEyes/MaidenWithEyesOfBlue/MaidenWithEyesOfBlueEffect.h"

// Dichiarazioni anticipate per le classi usate da main
class DrawController;
class DiscardController;
class Dec

namespace GameWiring {

// Registra un piccolo insieme di effetti/attivazioni di default usati nei demo di gioco.
void registerDefaultEffects(Game &game, TextureManager& tm);

// Crea una nuova istanza di Game e attacca controller/risorse. Centralizza
// la logica di inizializzazione ripetuta usata in più punti di main.
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
    std::optional<Card> &p2FieldSpellCard,
    TextureManager &textureManager
);

// Funzione di utilità per popolare AppHandlers::Context e attaccare i gestori per un gioco avviato.
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
    std::function<void(const std::vector<size_t>&)> setDeckSendIndexMap,
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

}