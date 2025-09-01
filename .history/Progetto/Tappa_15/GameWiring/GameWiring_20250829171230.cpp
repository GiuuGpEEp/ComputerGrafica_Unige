#include "GameWiring.h"

namespace GameWiring {

void registerDefaultEffects(Game &game){
    game.registerEffectForCardName("Pietra Bianca della Leggenda", std::make_unique<WhiteStoneLegendEffect>());
    game.registerEffectForCardName("Pietra Bianca degli Antichi", std::make_unique<WhiteStoneAncientsEffect>());
    // Activation effects for spells
    game.registerActivationForCardName("Reliquario del Drago", std::make_unique<ReliquarioDelDragoEffect>());
    game.registerActivationForCardName("Pianto di Argento", std::make_unique<PiantoDiArgentoEffect>());
}

void createGameAndAttachResources(
    const std::string &selectedDeckName,
    Deck &deck,
    Deck &opponentDeck,
    ResourceManager & /*resourceManager*/,
    std::unique_ptr<Game> &game,
    DrawController &drawController,
    AIController &aiP2,
    std::vector<Card>* &handPtr,
    std::vector<Card> &p1STCards,
    std::vector<Card> &p2STCards,
    std::optional<Card> &p1FieldSpellCard,
    std::optional<Card> &p2FieldSpellCard
){
    // Si presuppone che `deck` e `opponentDeck` siano già inizializzati dal chiamante
    Player p1("Player1", deck);
    Player p2("Player2", opponentDeck);
    game = std::make_unique<Game>(p1, p2);
    aiP2.attachGame(game.get());
    // Clear ST/FieldSpell UI caches
    p1STCards.clear();
    p2STCards.clear();
    p1FieldSpellCard.reset();
    p2FieldSpellCard.reset();
    // Registra gli effetti e collega controller / deck
    registerDefaultEffects(*game);
    game->attachExternalDeck(&deck, 0);
    game->attachExternalDeck(&opponentDeck, 1);
    game->attachDrawController(&drawController);
    // Assicura che il puntatore alla mano visibile faccia riferimento al Player assoluto 0
    if(game){
        int cur = game->getTurn().getCurrentPlayerIndex();
        handPtr = (cur==0) ? &game->current().getHand() : &game->opponent().getHand();
        if(handPtr) handPtr->clear();
    }
}

// Helper per popolare AppHandlers::Context e collegare gli handler per una partita avviata.
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
){
    appHandlersCtx.syncZones = [syncZones](){ if(syncZones) syncZones(); };
    appHandlersCtx.syncMonsterZoneToField = [syncMonsterZoneToField](){ if(syncMonsterZoneToField) syncMonsterZoneToField(); };
    appHandlersCtx.syncSpellTrapZones = [syncSpellTrapZones](){ if(syncSpellTrapZones) syncSpellTrapZones(); };
    appHandlersCtx.updateHandLayout = [updateHandLayout](){ if(updateHandLayout) updateHandLayout(); };
    appHandlersCtx.openResponsePromptIfAny = [openResponsePromptIfAny](){ if(openResponsePromptIfAny) openResponsePromptIfAny(); };
    appHandlersCtx.rebuildResponseActivables = [rebuildResponseActivables](){ if(rebuildResponseActivables) rebuildResponseActivables(); };
    appHandlersCtx.closeResponsePrompt = [closeResponsePrompt](){ if(closeResponsePrompt) closeResponsePrompt(); };
    appHandlersCtx.stopAllInputActions = [stopAllInputActions](){ if(stopAllInputActions) stopAllInputActions(); };
    appHandlersCtx.resetDeckPositions = resetDeckPositions;
    appHandlersCtx.setSelectingTributes = setSelectingTributes;
    appHandlersCtx.setTributesNeeded = setTributesNeeded;
    appHandlersCtx.setSsChoiceActive = setSsChoiceActive;
    appHandlersCtx.setDeckSendChoiceActive = setDeckSendChoiceActive;
    appHandlersCtx.setDeckSendCandidates = setDeckSendCandidates;
    appHandlersCtx.setDeckSendOwner = setDeckSendOwner;
    appHandlersCtx.pushBattleFeedbackMsg = pushBattleFeedbackMsg;
    appHandlersCtx.drawStartSubscriptionId = drawStartSubscriptionId;
    appHandlersCtx.drawEndSubscriptionId = drawEndSubscriptionId;
    appHandlersCtx.attackDeclaredSubId = attackDeclaredSubId;
    appHandlersCtx.attackResolvedSubId = attackResolvedSubId;
    appHandlersCtx.monsterDestroyedSubId = monsterDestroyedSubId;
    appHandlersCtx.lifeChangedSubId = lifeChangedSubId;
    appHandlersCtx.directAttackSubId = directAttackSubId;

    if(game){
        AppHandlers::attachGameHandlers(*game, appHandlersCtx);
    // imposta il limite mano e la callback per lo scarto (usa lo stesso limite di main: 7)
    game->setHandLimit(7);
        game->setDiscardCallback([&](std::vector<Card>&& excess){
            if(!handPtr) return;
            if(excess.empty()) return;
            sf::Vector2f gyPos = field.getSlotPosition(Type::Graveyard, 1);
            sf::Vector2f gyCenter(gyPos.x + slotSize.x*0.5f, gyPos.y + slotSize.y*0.5f);
            discardController.start(std::move(excess), gyCenter);
            if(updateHandLayout) updateHandLayout();
            std::cout << "[AutoDiscard] Scarto animato avviato." << std::endl;
        });
    }
}

} // namespace GameWiring
