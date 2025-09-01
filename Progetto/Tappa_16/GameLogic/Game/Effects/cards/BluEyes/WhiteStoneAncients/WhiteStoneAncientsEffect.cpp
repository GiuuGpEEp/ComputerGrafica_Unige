#include "WhiteStoneAncientsEffect.h"


void WhiteStoneAncientsEffect::onEvent(GameEventType type, Game& game){
    // 1) Traccia quando la carta viene mandata al cimitero in questo turno.
    //    L'effetto deve attivarsi indipendentemente dal fatto che la carta sia l'ultima mandata.
    if(type == GameEventType::CardSentToGrave){
        auto last = game.getLastSentToGrave();
        if(!last.has_value()) return;
        if(last->name == std::string("Pietra Bianca degli Antichi")){
            // Preferiamo l'owner registrato in lastSentToGrave (più affidabile di una ricerca per nome)
            int actualOwner = last->ownerIdx;
            // Verifica che il cimitero registrato contenga effettivamente la carta
            bool foundInRecorded = false;
            if(actualOwner >= 0 && actualOwner <= 1){
                auto &gyRec = game.getGraveyardOf(actualOwner);
                for(const auto &c : gyRec){ if(c.getName() == last->name){ foundInRecorded = true; break; } }
            }
            if(!foundInRecorded){
                // Se non presente dove registrato, effettua una scansione difensiva di entrambi i cimiteri
                actualOwner = -1;
                for(int i=0;i<2;++i){
                    auto &gy = game.getGraveyardOf(i);
                    for(const auto &c : gy){ if(c.getName() == last->name){ actualOwner = i; break; } }
                    if(actualOwner >= 0) break;
                }
                // Se ancora non trovato, torna al proprietario registrato
                if(actualOwner < 0) actualOwner = last->ownerIdx;
            }
            // Segnala che durante questo turno c'è stato l'invio della Pietra Bianca: l'End Phase dovrà processare la Special Summon
            pendingThisTurn = true;
            ownerIdx = actualOwner;
        }
        return;
    }

    // 2) All'inizio del turno successivo, resettiamo lo stato
    if(type == GameEventType::TurnStart){
        pendingThisTurn = false;
        ownerIdx = -1;
        return;
    }

    // 3) Alla End Phase dello stesso turno in cui la carta è stata mandata al cimitero, esegui la Special Summon
    if(type == GameEventType::PhaseChange){
        if(!pendingThisTurn) return;
        if(game.getTurn().getPhase() != GamePhase::End) return; // solo quando entriamo in End
        if(ownerIdx < 0) return;
        Deck &deck = game.getDeckOf(ownerIdx);
        auto found = deck.removeFirstByName("Drago Bianco Occhi Blu");
        if(found.has_value()){
            Deck &deckRef = const_cast<Deck&>(game.getDeckOf(ownerIdx));
            // Mantieni in sync anche il Deck esterno UI (passando il deck sorgente per evitare double-remove)
            game.mirrorExternalDeckRemoveByName("Drago Bianco Occhi Blu", &deckRef);
            bool ok = game.requestSpecialSummonWithChoice(ownerIdx, std::move(*found));
            (void)ok; // ignoriamo il log di debug
        }
        // Consuma il pending (one-shot)
        pendingThisTurn = false;
        ownerIdx = -1;
        return;
    }
}
