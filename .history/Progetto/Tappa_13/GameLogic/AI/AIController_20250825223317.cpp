#include "AIController.h"
#include "../Game/Game.h"
#include <algorithm>

void AIController::resetPhaseState(){
    actedInThisPhase_ = false;
}

void AIController::onNewPhase(){
    actedInThisPhase_ = false;
}

std::optional<size_t> AIController::findFirstFreeMonsterSlot() const {
    if(!game_) return std::nullopt;
    for(size_t s=0; s<3; ++s){
        if(!game_->isMonsterSlotOccupied(aiPlayerIdx_, s)) return s;
    }
    return std::nullopt;
}

std::optional<size_t> AIController::pickSummonFromHand() const {
    if(!game_) return std::nullopt;
    if(game_->getTurn().getCurrentPlayerIndex() != aiPlayerIdx_) return std::nullopt;
    const auto& hand = game_->current().getHand();
    for(size_t i=0;i<hand.size();++i){
        const auto& c = hand[i];
        if(c.getType() == Type::Monster){
            auto lvl = c.getLevelOrRank();
            if(lvl.has_value() && *lvl <= 4) return i;
        }
    }
    return std::nullopt;
}

std::optional<size_t> AIController::pickNormalSpellFromHand() const {
    if(!game_) return std::nullopt;
    if(game_->getTurn().getCurrentPlayerIndex() != aiPlayerIdx_) return std::nullopt;
    const auto& hand = game_->current().getHand();
    for(size_t i=0;i<hand.size();++i){
        const auto& c = hand[i];
        if(c.getType() == Type::SpellTrap || c.getType() == Type::FieldSpell){
            return i; // lascia a Game il gating (le Trappole dalla mano falliranno)
        }
    }
    return std::nullopt;
}

void AIController::tryActivateNormalSpells(){
    if(!game_) return;
    if(game_->getTurn().getCurrentPlayerIndex() != aiPlayerIdx_) return;
    if(auto idx = pickNormalSpellFromHand()){
        game_->activateSpellFromHand(*idx);
    }
}

void AIController::trySetTrapsAndQuick(){
    if(!game_) return;
    if(game_->getTurn().getCurrentPlayerIndex() != aiPlayerIdx_) return;
    auto& hand = game_->current().getHand();
    for(size_t i=0; i<hand.size();){
        bool progressed = false;
        if(hand[i].getType() == Type::FieldSpell){
            if(game_->setFieldSpell(i)) progressed = true;
        } else if(hand[i].getType() == Type::SpellTrap){
            if(game_->setSpellOrTrap(i)) progressed = true;
        }
        if(progressed){ /* la mano si compatta, non incrementare i */ }
        else { ++i; }
    }
}

std::optional<size_t> AIController::pickBestAttackTarget() const {
    if(!game_) return std::nullopt;
    const auto& opp = game_->getOpponentMonsterZone();
    if(opp.empty()) return std::nullopt; // attacco diretto
    size_t best = 0; int bestAtk = opp[0].getValues().has_value() ? opp[0].getValues()->first : 0;
    for(size_t i=0;i<opp.size();++i){
        int atk = opp[i].getValues().has_value() ? opp[i].getValues()->first : 0;
        if(atk < bestAtk){ bestAtk = atk; best = i; }
    }
    return best;
}

void AIController::doMain1(){
    if(actedInThisPhase_) return;
    // 1) Prova evocazione semplice in slot libero piu' a sinistra
    if(auto slot = findFirstFreeMonsterSlot()){
        if(auto handIdx = pickSummonFromHand()){
            if(game_->tryNormalSummonToSlot(*handIdx, *slot)){
                actedInThisPhase_ = true;
                return;
            }
        }
    }
    // 2) Prova ad attivare una Magia dalla mano
    tryActivateNormalSpells();
    actedInThisPhase_ = true;
}

void AIController::doBattle(){
    if(actedInThisPhase_) return;
    const auto& mz = game_->getMonsterZone();
    for(size_t i=0;i<mz.size();++i){
        if(!game_->hasMonsterAlreadyAttacked(i)){
            auto tgt = pickBestAttackTarget();
            if(game_->canDeclareAttack(i, tgt)){
                game_->declareAttack(i, tgt);
            }
        }
    }
    actedInThisPhase_ = true;
}

void AIController::doMain2(){
    if(actedInThisPhase_) return;
    trySetTrapsAndQuick();
    actedInThisPhase_ = true;
}

void AIController::update(float){
    if(!game_) return;
    if(game_->getTurn().getCurrentPlayerIndex() != aiPlayerIdx_) return;

    int tc = (int)game_->getTurn().getTurnCount();
    int ph = (int)game_->getTurn().getPhase();
    if(tc != lastTurnCount_ || ph != lastPhase_){
        lastTurnCount_ = tc;
        lastPhase_ = ph;
        onNewPhase();
    }

    switch(game_->getTurn().getPhase()){
        case GamePhase::Draw:
        case GamePhase::Standby:
            break;
        case GamePhase::Main1:
            doMain1();
            break;
        case GamePhase::Battle:
            doBattle();
            break;
        case GamePhase::Main2:
            doMain2();
            break;
        case GamePhase::End:
            if(!game_->shouldAutoEndTurn() && !game_->hasPendingSpecialSummon()){
                game_->endTurn();
            }
            break;
    }

    if(actedInThisPhase_){
        game_->advancePhase();
    }
}
