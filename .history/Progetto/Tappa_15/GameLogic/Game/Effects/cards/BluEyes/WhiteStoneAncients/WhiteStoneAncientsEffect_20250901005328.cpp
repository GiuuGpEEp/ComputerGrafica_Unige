#include "WhiteStoneAncientsEffect.h"


void WhiteStoneAncientsEffect::onEvent(GameEventType type, Game& game){
    // 1) Track quando viene mandata al Cimitero in questo turno
    if(type == GameEventType::CardSentToGrave){
        auto last = game.getLastSentToGrave();
        if(!last.has_value()) return;
        if(last->name == std::string("Pietra Bianca degli Antichi")){
            // Prefer the recorded owner from lastSentToGrave (more reliable than name-match across graveyards)
            int actualOwner = last->ownerIdx;
            // Validate that the recorded owner's graveyard contains the sent card (search all entries)
            bool foundInRecorded = false;
            if(actualOwner >= 0 && actualOwner <= 1){
                auto &gyRec = game.getGraveyardOf(actualOwner);
                for(const auto &c : gyRec){ if(c.getName() == last->name){ foundInRecorded = true; break; } }
            }
            if(!foundInRecorded){
                // If not found where recorded, attempt a defensive scan of both graveyards by name
                actualOwner = -1;
                for(int i=0;i<2;++i){
                    auto &gy = game.getGraveyardOf(i);
                    for(const auto &c : gy){ if(c.getName() == last->name){ actualOwner = i; break; } }
                    if(actualOwner >= 0) break;
                }
                // If still not found, fall back to recorded owner
                if(actualOwner < 0) actualOwner = last->ownerIdx;
            }
            pendingThisTurn = true;
            ownerIdx = actualOwner;
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
        // Debug: report deck info before searching
        std::cout << "[Effect] WSoAncients searching deck of player " << ownerIdx << ", size=" << deck.getSize() << std::endl;
        // Print up to first 8 card names for inspection
        auto all = deck.collectWhere([](const Card&){ return true; });
        for(size_t i=0;i<all.size() && i<8; ++i){
            std::cout << "[Effect] WSoAncients deck[" << i << "]='" << all[i].getName() << "'\n";
        }
        auto found = deck.removeFirstByName("Drago Bianco Occhi Blu");
            if(found.has_value()){
                Deck &deckRef = const_cast<Deck&>(game.getDeckOf(ownerIdx));
                // Mantieni in sync anche il Deck esterno UI (passando il deck sorgente per evitare double-remove)
                game.mirrorExternalDeckRemoveByName("Drago Bianco Occhi Blu", &deckRef);
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
