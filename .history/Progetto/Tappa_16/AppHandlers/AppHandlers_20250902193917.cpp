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

    // Gestione: scelta Aggiunta dal Deck (Melodia)
    g.events().subscribe(GameEventType::DeckAddChoiceRequested, [&](){
        // Chiudi il prompt di risposta, stiamo entrando in un flusso modale di scelta dal Deck
        if(ctx.closeResponsePrompt) ctx.closeResponsePrompt();
        auto ownerOpt = g.getPendingAddOwner();
        int owner = ownerOpt.has_value() ? *ownerOpt : g.getTurn().getCurrentPlayerIndex();
        auto modeOpt = g.getPendingAddMode();
        // AI auto-pick
        if(owner == 1){
            const Deck &d = g.getDeckOf(owner);
            auto candidates = d.collectWhere([&](const Card& c){
                if(modeOpt.has_value() && *modeOpt == Game::AddChoiceMode::Level1Dragon){
                    if(c.getType() != Type::Monster) return false;
                    const auto &feats = c.getFeatures();
                    bool isDragon = std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
                    auto lv = c.getLevelOrRank();
                    return isDragon && lv.has_value() && *lv == 1;
                } else {
                    auto val = c.getValues(); if(!val.has_value()) return false; int atk=val->first, def=val->second;
                    if(atk < 3000 || def > 2500) return false; const auto &feats = c.getFeatures();
                    return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
                }
            });
            if(candidates.empty()){ g.cancelPendingAddFromDeck(); if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Nessuna carta valida nel Deck"); return; }
            std::uniform_int_distribution<int> dist(0, static_cast<int>(candidates.size())-1);
            int chosen = dist(_apphandlers_rand);
            (void)g.resolvePendingAddFromDeck(static_cast<size_t>(chosen));
            return;
        }
        // Human: riusa overlay DeckSend (testo resterà quello standard)
        if(ctx.setDeckSendChoiceActive) ctx.setDeckSendChoiceActive(true);
        if(ctx.setDeckSendCandidates){
            const Deck &d = g.getDeckOf(owner);
            auto candidates = d.collectWhere([&](const Card& c){
                if(modeOpt.has_value() && *modeOpt == Game::AddChoiceMode::Level1Dragon){
                    if(c.getType() != Type::Monster) return false;
                    const auto &feats = c.getFeatures();
                    bool isDragon = std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
                    auto lv = c.getLevelOrRank();
                    return isDragon && lv.has_value() && *lv == 1;
                } else {
                    auto val = c.getValues(); if(!val.has_value()) return false; int atk=val->first, def=val->second;
                    if(atk < 3000 || def > 2500) return false; const auto &feats = c.getFeatures();
                    return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
                }
            });
            ctx.setDeckSendCandidates(candidates);
            if(candidates.empty()){
                g.cancelPendingAddFromDeck();
                if(ctx.setDeckSendChoiceActive) ctx.setDeckSendChoiceActive(false);
                if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Nessuna carta valida nel Deck");
                return;
            }
            if(ctx.stopAllInputActions) ctx.stopAllInputActions();
        }
    });

    // Gestione: scelta scarto dalla Mano (Melodia)
    g.events().subscribe(GameEventType::HandDiscardChoiceRequested, [&](){
        // Chiudi l'eventuale response window per permettere i click sulla mano
        if(ctx.closeResponsePrompt) ctx.closeResponsePrompt();
        auto ownerOpt = g.getPendingHandDiscardOwner();
        int owner = ownerOpt.has_value() ? *ownerOpt : g.getTurn().getCurrentPlayerIndex();
        // AI: scarta casualmente
        if(owner == 1){
            auto &hand = g.getHandOf(owner);
            if(hand.empty()){ g.cancelPendingHandDiscard(); return; }
            std::uniform_int_distribution<int> dist(0, static_cast<int>(hand.size())-1);
            int idx = dist(_apphandlers_rand);
            (void)g.resolvePendingHandDiscard(static_cast<size_t>(idx));
            return;
        }
        // Human: non abbiamo un overlay dedicato per la mano; semplifichiamo scartando l'ultima selezionata via input principali.
        // Come fallback apriamo un messaggio e aspettiamo che l'utente clicchi una carta in mano (gestito nel main tramite action context)
        if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Seleziona una carta in mano da scartare");
        if(ctx.stopAllInputActions) ctx.stopAllInputActions();
        // Nota: l'effettiva conferma avverrà quando il main chiamerà game.resolvePendingHandDiscard(index) dal click sulla mano.
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

    // Gestione: scelta dal Cimitero (Richiamo del Posseduto)
    g.events().subscribe(GameEventType::GraveyardChoiceRequested, [&](){
        // Chiudi il prompt di risposta mentre apriamo una modale
        if(ctx.closeResponsePrompt) ctx.closeResponsePrompt();
        auto ownerOpt = g.getPendingGraveyardOwner();
        int owner = ownerOpt.has_value() ? *ownerOpt : g.getTurn().getCurrentPlayerIndex();
        auto &gy = g.getGraveyardOf(owner);
        // AI: selezione casuale tra i mostri
        if(owner == 1){
            std::vector<size_t> idxs; idxs.reserve(gy.size());
            for(size_t i=0;i<gy.size();++i){ if(gy[i].getType()==Type::Monster) idxs.push_back(i); }
            if(idxs.empty()){ g.cancelPendingSelectFromGraveyard(); if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Nessun mostro nel Cimitero"); return; }
            std::uniform_int_distribution<int> dist(0, static_cast<int>(idxs.size())-1);
            size_t pick = idxs[dist(_apphandlers_rand)];
            (void)g.resolvePendingSelectFromGraveyard(pick);
            return;
        }
        // Human: riusa il pannello di scelta del Deck con i contenuti del Cimitero del proprietario
        if(ctx.setDeckSendChoiceActive) ctx.setDeckSendChoiceActive(true);
        if(ctx.setDeckSendOwner) ctx.setDeckSendOwner(owner);
        if(ctx.setDeckSendCandidates){
            // Filtra solo i mostri, come da effetto semplificato
            std::vector<Card> candidates;
            candidates.reserve(gy.size());
            std::vector<size_t> indexMap;
            indexMap.reserve(gy.size());
            for(size_t i=0;i<gy.size();++i){ if(gy[i].getType()==Type::Monster){ candidates.push_back(gy[i]); indexMap.push_back(i);} }
            ctx.setDeckSendCandidates(candidates);
            if(ctx.setDeckSendIndexMap) ctx.setDeckSendIndexMap(indexMap);
            if(candidates.empty()){
                g.cancelPendingSelectFromGraveyard();
                if(ctx.setDeckSendChoiceActive) ctx.setDeckSendChoiceActive(false);
                if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Nessun mostro nel Cimitero");
                return;
            }
            if(ctx.stopAllInputActions) ctx.stopAllInputActions();
        }
    });

    // Gestione: scelta di un proprio mostro (per costi/effetti come Saggio)
    g.events().subscribe(GameEventType::OwnMonsterChoiceRequested, [&](){
        if(ctx.closeResponsePrompt) ctx.closeResponsePrompt();
        auto ownerOpt = g.getPendingOwnMonsterOwner();
        int owner = ownerOpt.has_value() ? *ownerOpt : g.getTurn().getCurrentPlayerIndex();
        if(owner == 1){
            // AI: sceglie casualmente uno dei suoi mostri sul campo
            const auto &mz = g.getMonsterZone();
            if(mz.empty()){ g.cancelPendingSelectOwnMonster(); return; }
            std::uniform_int_distribution<int> dist(0, static_cast<int>(mz.size())-1);
            size_t choice = static_cast<size_t>(dist(_apphandlers_rand));
            (void)g.resolvePendingSelectOwnMonster(choice);
            return;
        }
        // Human: mostra un messaggio e blocca gli altri input; il click sul campo verrà gestito dal main/InputController
        if(ctx.pushBattleFeedbackMsg) ctx.pushBattleFeedbackMsg("Seleziona un tuo mostro da mandare al Cimitero");
        if(ctx.stopAllInputActions) ctx.stopAllInputActions();
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
