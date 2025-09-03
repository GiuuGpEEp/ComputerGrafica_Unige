#include "AppHandlers.h"
#include <random>


using namespace AppHandlers;

// file-scope RNG for AI decisions (ensure declared before handlers use it)
static std::mt19937 _apphandlers_rand(std::random_device{}());

void AppHandlers::attachGameHandlers(Game &g, const Context &ctx){
    if(ctx.drawStartSubscriptionId) *ctx.drawStartSubscriptionId = g.events().subscribe(GameEventType::DrawStart, [](){
        std::cout << "[Event] DrawStart" << std::endl;
    });
    if(ctx.drawEndSubscriptionId) *ctx.drawEndSubscriptionId = g.events().subscribe(GameEventType::DrawEnd, [](){
        std::cout << "[Event] DrawEnd" << std::endl;
    });

    g.events().subscribe(GameEventType::NormalSummonTributeRequired, [&](){
    std::cout << "[AppHandlers] NormalSummonTributeRequired handler start" << std::endl;
    if(ctx.setSelectingTributes) ctx.setSelectingTributes(true);
    if(ctx.setTributesNeeded) ctx.setTributesNeeded(g.getPendingTributesNeeded());
    if(ctx.stopAllInputActions) ctx.stopAllInputActions();
    std::cout << "[AppHandlers] NormalSummonTributeRequired handler end" << std::endl;
    });

    g.events().subscribe(GameEventType::MonstersTributed, [&](){ if(ctx.syncMonsterZoneToField) ctx.syncMonsterZoneToField(); });

    g.events().subscribe(GameEventType::NormalSummon, [&](){ if(ctx.syncZones) ctx.syncZones(); if(ctx.openResponsePromptIfAny) ctx.openResponsePromptIfAny(); });
    // Strumento diagnostico: stampa quando viene emesso l'evento NormalSummon
    g.events().subscribe(GameEventType::NormalSummon, [&](){ std::cout << "[AppHandlers] NormalSummon fired" << std::endl; if(ctx.syncZones) ctx.syncZones(); if(ctx.openResponsePromptIfAny) ctx.openResponsePromptIfAny(); });
    g.events().subscribe(GameEventType::NormalSet, [&](){ if(ctx.syncMonsterZoneToField) ctx.syncMonsterZoneToField(); if(ctx.syncSpellTrapZones) ctx.syncSpellTrapZones(); });

    g.events().subscribe(GameEventType::TurnStart, [&](){
        if(ctx.syncZones) ctx.syncZones();
        if(ctx.updateHandLayout) ctx.updateHandLayout();
        if(ctx.openResponsePromptIfAny) ctx.openResponsePromptIfAny();
    });

    g.events().subscribe(GameEventType::PhaseChange, [&](){ if(ctx.openResponsePromptIfAny) ctx.openResponsePromptIfAny(); });

    g.events().subscribe(GameEventType::TurnEnd, [&](){ if(ctx.syncZones) ctx.syncZones(); if(ctx.openResponsePromptIfAny) ctx.openResponsePromptIfAny(); });

    if(ctx.attackDeclaredSubId) *ctx.attackDeclaredSubId = g.events().subscribe(GameEventType::AttackDeclared, [&](){ if(ctx.openResponsePromptIfAny) ctx.openResponsePromptIfAny(); });
    if(ctx.attackResolvedSubId) *ctx.attackResolvedSubId = g.events().subscribe(GameEventType::AttackResolved, [&](){ if(ctx.syncMonsterZoneToField) ctx.syncMonsterZoneToField(); });
    if(ctx.monsterDestroyedSubId) *ctx.monsterDestroyedSubId = g.events().subscribe(GameEventType::MonsterDestroyed, [&](){ /* noop for now */ });
    if(ctx.lifeChangedSubId) *ctx.lifeChangedSubId = g.events().subscribe(GameEventType::LifePointsChanged, [&](){ if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("LP cambiati"); });

    g.events().subscribe(GameEventType::CardMoved, [&](){
    std::cout << "[AppHandlers] CardMoved" << std::endl;
    if(ctx.syncZones) ctx.syncZones();
    if(ctx.resetDeckPositions) ctx.resetDeckPositions();
    if(ctx.updateHandLayout) ctx.updateHandLayout();
    });

    g.events().subscribe(GameEventType::MonsterFlipped, [&](){ if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Flip!"); if(ctx.syncMonsterZoneToField) ctx.syncMonsterZoneToField(); if(ctx.syncSpellTrapZones) ctx.syncSpellTrapZones(); });

    g.events().subscribe(GameEventType::SpellSet, [&](){ if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Magia settata"); if(ctx.syncSpellTrapZones) ctx.syncSpellTrapZones(); });
    g.events().subscribe(GameEventType::TrapSet, [&](){ if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Trappola settata"); if(ctx.syncSpellTrapZones) ctx.syncSpellTrapZones(); });

    g.events().subscribe(GameEventType::SpellActivated, [&](){
        if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Magia attivata");
        if(ctx.syncSpellTrapZones) ctx.syncSpellTrapZones();
        // Chain window: dopo ogni attivazione prova ad aprire/aggiornare il prompt di risposta
        if(ctx.rebuildResponseActivables) ctx.rebuildResponseActivables();
        if(ctx.openResponsePromptIfAny) ctx.openResponsePromptIfAny();
    });
    g.events().subscribe(GameEventType::TrapActivated, [&](){
        if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Trappola attivata");
        if(ctx.syncSpellTrapZones) ctx.syncSpellTrapZones();
        if(ctx.rebuildResponseActivables) ctx.rebuildResponseActivables();
        if(ctx.openResponsePromptIfAny) ctx.openResponsePromptIfAny();
    });

    g.events().subscribe(GameEventType::DeckSendChoiceRequested, [&](){
        // Decide owner of pending send; prefer Game's pendingSend owner if present
        auto pendingOwnerOpt = g.getPendingSendOwner();
        int owner = pendingOwnerOpt.has_value() ? *pendingOwnerOpt : g.getTurn().getCurrentPlayerIndex();
        
        // If owner is AI (1), auto-resolve choose a dragon randomly without opening the UI
        if(owner == 1){
            const Deck &d = g.getDeckOf(owner);
            auto candidates = d.collectWhere(
                [](const Card& c){ 
                    const auto &feats = c.getFeatures(); 
                    return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end(); 
                });
            if(candidates.empty()){
                g.cancelPendingSendFromDeck();
                if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Nessun Drago nel Deck");
                return;
            }
            // choose random candidate index
            std::uniform_int_distribution<int> dist(0, static_cast<int>(candidates.size())-1);
            int chosen = dist(_apphandlers_rand);
            (void)g.resolvePendingSendFromDeck(static_cast<size_t>(chosen));
            return;
        }

        // Otherwise (owner is human) open the UI panel as before
        if(ctx.setDeckSendChoiceActive) ctx.setDeckSendChoiceActive(true);
        if(ctx.setDeckSendChoiceActive){
            const Deck &d = g.getDeckOf(owner);
            auto candidates = d.collectWhere([](const Card& c){ 
                const auto &feats = c.getFeatures(); 
                return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end(); 
            });
            if(ctx.setDeckSendCandidates) ctx.setDeckSendCandidates(candidates);
            if(candidates.empty()){
                g.cancelPendingSendFromDeck();
                if(ctx.setDeckSendChoiceActive) ctx.setDeckSendChoiceActive(false);
                if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Nessun Drago nel Deck");
                return;
            }
            if(ctx.stopAllInputActions) ctx.stopAllInputActions();
        }
    });

    // Random per le decisioni del bot
    static std::mt19937 _apphandlers_rand(std::random_device{}());
    g.events().subscribe(GameEventType::SpecialSummonChoiceRequested, [&](){
    // Prefer reading the pendingSS owner from the Game to avoid relying on current turn index
    auto pendingOwnerOpt = g.getPendingSpecialSummonOwner();
    int owner = pendingOwnerOpt.has_value() ? *pendingOwnerOpt : g.getTurn().getCurrentPlayerIndex();
    std::cout << "[AppHandlers] SpecialSummonChoiceRequested owner=" << owner << std::endl;
    if(owner == 1){
        // AI: choose autonomously between Attack (false) and Defense (true).
        std::uniform_int_distribution<int> dist(0,1);
        bool chooseDefense = (dist(_apphandlers_rand) == 1);
        (void)g.resolvePendingSpecialSummon(chooseDefense);
    } else {
        if(ctx.setSsChoiceActive) ctx.setSsChoiceActive(true);
        if(ctx.stopAllInputActions) ctx.stopAllInputActions();
    }
    });

    if(ctx.directAttackSubId) *ctx.directAttackSubId = g.events().subscribe(GameEventType::DirectAttack, [](){ std::cout<<"[Event] DirectAttack"<<std::endl; });

    g.events().subscribe(GameEventType::Win, [&](){ if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Win"); });
    g.events().subscribe(GameEventType::Lose, [&](){ if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Lose"); });
}

void AppHandlers::detachGameHandlers(Game &g, const Context &ctx){
    // Annulla la sottoscrizione di eventuali id memorizzati
    if(ctx.drawStartSubscriptionId && *ctx.drawStartSubscriptionId){ g.events().unsubscribe(*ctx.drawStartSubscriptionId); *ctx.drawStartSubscriptionId = 0; }
    if(ctx.drawEndSubscriptionId && *ctx.drawEndSubscriptionId){ g.events().unsubscribe(*ctx.drawEndSubscriptionId); *ctx.drawEndSubscriptionId = 0; }
    if(ctx.attackDeclaredSubId && *ctx.attackDeclaredSubId){ g.events().unsubscribe(*ctx.attackDeclaredSubId); *ctx.attackDeclaredSubId = 0; }
    if(ctx.attackResolvedSubId && *ctx.attackResolvedSubId){ g.events().unsubscribe(*ctx.attackResolvedSubId); *ctx.attackResolvedSubId = 0; }
    if(ctx.monsterDestroyedSubId && *ctx.monsterDestroyedSubId){ g.events().unsubscribe(*ctx.monsterDestroyedSubId); *ctx.monsterDestroyedSubId = 0; }
    if(ctx.lifeChangedSubId && *ctx.lifeChangedSubId){ g.events().unsubscribe(*ctx.lifeChangedSubId); *ctx.lifeChangedSubId = 0; }
    if(ctx.directAttackSubId && *ctx.directAttackSubId){ g.events().unsubscribe(*ctx.directAttackSubId); *ctx.directAttackSubId = 0; }
    // Nota: le altre sottoscrizioni create senza memorizzare id vengono lasciate inalterate; sono di breve durata o innocue.
}
