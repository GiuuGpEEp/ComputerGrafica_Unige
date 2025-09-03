#include "GameWiring.h"

namespace GameWiring {

// Registra gli effetti di default delle carte nel gioco
void registerDefaultEffects(Game &game, TextureManager& tm){
    game.registerEffectForCardName("Pietra Bianca della Leggenda", std::make_unique<WhiteStoneLegendEffect>(tm));
    game.registerEffectForCardName("Pietra Bianca degli Antichi", std::make_unique<WhiteStoneAncientsEffect>());
    game.registerEffectForCardName("Saggio con Occhi di Blu", std::make_unique<SageWithEyesOfBlueEffect>());
    game.registerActivationForCardName("Saggio con Occhi di Blu", std::make_unique<SageWithEyesOfBlueActivationEffect>());
    // Effetti di attivazione per le magie
    game.registerActivationForCardName("Reliquario del Drago", std::make_unique<ReliquarioDelDragoEffect>());
    game.registerActivationForCardName("Pianto di Argento", std::make_unique<PiantoDiArgentoEffect>());
    game.registerActivationForCardName("La Melodia del Drago che si Risveglia", std::make_unique<LaMelodiaDelDragoCheSiRisvegliaEffect>());
    game.registerActivationForCardName("Richiamo del Posseduto", std::make_unique<RichiamoDelPossedutoEffect>());
    game.registerActivationForCardName("Rivali Predestinati", std::make_unique<RivaliPredestinatiEffect>());
}

// Crea il gioco e collega le risorse necessarie
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
    ,
    TextureManager &textureManager
){
    // Si presuppone che `deck` e `opponentDeck` siano già inizializzati dal chiamante
    Player p1("Player1", deck);
    Player p2("Player2", opponentDeck);
    game = std::make_unique<Game>(p1, p2);
    aiP2.attachGame(game.get());
    // Pulisci le cache UI di ST/FieldSpell
    p1STCards.clear();
    p2STCards.clear();
    p1FieldSpellCard.reset();
    p2FieldSpellCard.reset();
    // Registra gli effetti e collega controller / deck
    registerDefaultEffects(*game, textureManager);
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
    appHandlersCtx.setDeckSendIndexMap = setDeckSendIndexMap;
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
            // Determine the owner (absolute player index) whose hand produced the excess (0 = P1, 1 = P2)
            int ownerIdx = game->getTurn().getCurrentPlayerIndex();
            // Field::getSlotPosition expects player parameter = 1 for bottom (P1) and 2 for top (P2).
            int fieldPlayer = ownerIdx + 1; // convert 0->1, 1->2
            // Compute the graveyard slot position for that owner
            sf::Vector2f gyPos = field.getSlotPosition(Type::Graveyard, fieldPlayer);
            sf::Vector2f gyCenter(gyPos.x + slotSize.x*0.5f, gyPos.y + slotSize.y*0.5f);
            // Start discard animation targeting the specific owner's graveyard
            discardController.start(std::move(excess), gyCenter, ownerIdx);
            if(updateHandLayout) updateHandLayout();
            std::cout << "[AutoDiscard] Scarto animato avviato for owner: " << ownerIdx << std::endl;
        });
    }
}

} // namespace GameWiring
