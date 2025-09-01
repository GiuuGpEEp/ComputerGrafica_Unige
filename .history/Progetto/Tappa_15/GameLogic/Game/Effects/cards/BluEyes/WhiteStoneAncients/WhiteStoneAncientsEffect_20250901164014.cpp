#include "WhiteStoneAncientsEffect.h"


void WhiteStoneAncientsEffect::onEvent(GameEventType type, Game& game){
    // 1) Track quando viene mandata al Cimitero in questo turno
    if(type == GameEventType::CardSentToGrave){
        auto last = game.getLastSentToGrave();
        if(!last.has_value()) return;
        if(last->name == std::string("Pietra Bianca degli Antichi")){
            pendingThisTurn = true;
            ownerIdx = last->ownerIdx;
            std::cout << "[Effect] WSoAncients pending for End Phase, owner=" << ownerIdx << std::endl;
        }
        return;
    }

    // 2) All'inizio del turno successivo, resettiamo lo stato
    if(type == GameEventType::TurnStart){
        if(pendingThisTurn){
            std::cout << "[Effect] WSoAncients cleared at TurnStart" << std::endl;
        }
        pendingThisTurn = false;
        ownerIdx = -1;
        return;
    }

    // 3) Alla End Phase dello stesso turno in cui è stata mandata al Cimitero, effettuiamo la Special Summon
    if(type == GameEventType::PhaseChange){
        if(!pendingThisTurn) return;
        if(game.getTurn().getPhase() != GamePhase::End) return; // solo quando entriamo in End
        if(ownerIdx < 0) return;
        std::cout << "[Effect] WSoAncients -> End Phase trigger for owner=" << ownerIdx << std::endl;
        Deck &deck = game.getDeckOf(ownerIdx);
        auto found = deck.removeFirstByName("Drago Bianco Occhi Blu");
        if(found.has_value()){
            game.mirrorExternalDeckRemoveByName("Drago Bianco Occhi Blu");
            bool ok = game.requestSpecialSummonWithChoice(ownerIdx, std::move(*found));
            if(!ok){
                std::cout << "[Effect] WSoAncients: Request SS failed (zone full? pending?)" << std::endl;
            } else {
                std::cout << "[Effect] WSoAncients: Prompted SS choice (ATK/DEF)" << std::endl;
            }
        } else {
            std::cout << "[Effect] WSoAncients: BEWD not found in deck" << std::endl;
        }
        // Consuma il pending (one-shot)
        pendingThisTurn = false;
        ownerIdx = -1;
        return;
    }
}
