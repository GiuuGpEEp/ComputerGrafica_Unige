#include "AppHandlers.h"
#include "GameLogic/Game/Game.h"
#include "Deck/Deck.h"
#include "RenderUtils.h"
#include "Config.h"
#include <iostream>

using namespace AppHandlers;

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

    g.events().subscribe(GameEventType::NormalSummon, [&](){ if(ctx.syncZones) ctx.syncZones(); });
    // Instrument: print when NormalSummon fires
    g.events().subscribe(GameEventType::NormalSummon, [&](){ std::cout << "[AppHandlers] NormalSummon fired" << std::endl; if(ctx.syncZones) ctx.syncZones(); });
    g.events().subscribe(GameEventType::NormalSet, [&](){ if(ctx.syncMonsterZoneToField) ctx.syncMonsterZoneToField(); if(ctx.syncSpellTrapZones) ctx.syncSpellTrapZones(); });

    g.events().subscribe(GameEventType::TurnStart, [&](){
        if(ctx.syncZones) ctx.syncZones();
        if(ctx.updateHandLayout) ctx.updateHandLayout();
        if(ctx.openResponsePromptIfAny) ctx.openResponsePromptIfAny();
    });

    g.events().subscribe(GameEventType::PhaseChange, [&](){ if(ctx.openResponsePromptIfAny) ctx.openResponsePromptIfAny(); });

    g.events().subscribe(GameEventType::TurnEnd, [&](){ if(ctx.syncZones) ctx.syncZones(); });

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

    g.events().subscribe(GameEventType::SpellActivated, [&](){ if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Magia attivata"); if(ctx.syncSpellTrapZones) ctx.syncSpellTrapZones(); if(ctx.rebuildResponseActivables) { ctx.rebuildResponseActivables(); } if(ctx.closeResponsePrompt) { /* handled by caller if empty */ } });
    g.events().subscribe(GameEventType::TrapActivated, [&](){ if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Trappola attivata"); if(ctx.syncSpellTrapZones) ctx.syncSpellTrapZones(); if(ctx.rebuildResponseActivables) { ctx.rebuildResponseActivables(); } if(ctx.closeResponsePrompt) { /* handled by caller if empty */ } });

    g.events().subscribe(GameEventType::DeckSendChoiceRequested, [&](){
        if(ctx.setDeckSendChoiceActive) ctx.setDeckSendChoiceActive(true);
        if(ctx.setDeckSendChoiceActive){
            // gather candidates
            int owner = g.getTurn().getCurrentPlayerIndex();
            const Deck &d = g.getDeckOf(owner);
            auto candidates = d.collectWhere([](const Card& c){ const auto &feats = c.getFeatures(); return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end(); });
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

    g.events().subscribe(GameEventType::SpecialSummonChoiceRequested, [&](){
    int owner = g.getTurn().getCurrentPlayerIndex();
    std::cout << "[AppHandlers] SpecialSummonChoiceRequested owner=" << owner << std::endl;
    if(owner == 1){ (void)g.resolvePendingSpecialSummon(false); }
    else { if(ctx.setSsChoiceActive) ctx.setSsChoiceActive(true); if(ctx.stopAllInputActions) ctx.stopAllInputActions(); }
    });

    if(ctx.directAttackSubId) *ctx.directAttackSubId = g.events().subscribe(GameEventType::DirectAttack, [](){ std::cout<<"[Event] DirectAttack"<<std::endl; });

    g.events().subscribe(GameEventType::Win, [&](){ if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Win"); });
    g.events().subscribe(GameEventType::Lose, [&](){ if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Lose"); });
}
