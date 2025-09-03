#include "WhiteStoneAncientsEffect.h"


void WhiteStoneAncientsEffect::onEvent(GameEventType type, Game& game){
    // 1) Traccia quando la carta viene mandata al cimitero in questo turno (per giocatore)
    if(type == GameEventType::CardSentToGrave){
        auto last = game.getLastSentToGrave();
        if(!last.has_value()) return;
        if(last->name == std::string("Pietra Bianca degli Antichi")){
            int actualOwner = last->ownerIdx;
            // Convalida con una verifica nel cimitero previsto; in fallback scansiona entrambi.
            bool foundInRecorded = false;
            if(actualOwner >= 0 && actualOwner <= 1){
                auto &gyRec = game.getGraveyardOf(actualOwner);
                for(const auto &c : gyRec){ if(c.getName() == last->name){ foundInRecorded = true; break; } }
            }
            if(!foundInRecorded){
                for(int i=0;i<2;++i){
                    auto &gy = game.getGraveyardOf(i);
                    for(const auto &c : gy){ if(c.getName() == last->name){ actualOwner = i; foundInRecorded = true; break; } }
                    if(foundInRecorded) break;
                }
            }
            if(actualOwner >= 0 && actualOwner <= 1){
                pendingFor[actualOwner] = true; // marca pendente per quel giocatore
            }
        }
        return;
    }

    // 2) All'inizio del turno successivo, resettiamo lo stato per entrambi i giocatori
    if(type == GameEventType::TurnStart){
        pendingFor[0] = pendingFor[1] = false;
        return;
    }

    // 3) Alla End Phase dello stesso turno, processa entrambi i pending (se presenti)
    if(type == GameEventType::PhaseChange){
        if(game.getTurn().getPhase() != GamePhase::End) return; // solo quando entriamo in End
        bool any = pendingFor[0] || pendingFor[1];
        if(!any) return;

        // Ordine: per coerenza UX, risolvi prima il giocatore non umano (1) poi il giocatore umano (0),
    // in modo che gli auto-prompt dell'AI non interrompano l'interfaccia del P1.
        for(int owner : {1, 0}){
            if(!pendingFor[owner]) continue;

            Deck &deck = game.getDeckOf(owner);
            auto found = deck.removeFirstByName("Drago Bianco Occhi Blu");
            if(found.has_value()){
                Deck &deckRef = const_cast<Deck&>(game.getDeckOf(owner));
                game.mirrorExternalDeckRemoveByName("Drago Bianco Occhi Blu", &deckRef);
                (void)game.requestSpecialSummonWithChoice(owner, std::move(*found));
            }
        }

        // Consuma i pending (one-shot)
        pendingFor[0] = pendingFor[1] = false;
        return;
    }
}
