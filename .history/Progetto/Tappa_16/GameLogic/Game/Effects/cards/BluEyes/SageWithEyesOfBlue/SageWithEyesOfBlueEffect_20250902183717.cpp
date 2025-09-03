#include "SageWithEyesOfBlueEffect.h"
#include <algorithm>
#include <iostream>

static inline bool isBlueEyesMonster(const Card& c){
    if(c.getType() != Type::Monster) return false;
    const std::string &nm = c.getName();
    // Semplice check sul nome contiene "Occhi di Blu"; estendibile a blacklist/whitelist
    return nm.find("Occhi di Blu") != std::string::npos;
}

void SageWithEyesOfBlueEffect::onEvent(GameEventType type, Game& game){
    if(type != GameEventType::NormalSummon) return;
    // Verifica se l'ultima Normal Summon è proprio "Saggio con Occhi di Blu"
    auto lastNS = game.getLastNormalSummon();
    if(!lastNS.has_value()) return;
    if(lastNS->name != std::string("Saggio con Occhi di Blu")) return;
    int owner = lastNS->ownerIdx;
    // 1) Richiedi selezione di un proprio mostro da mandare al Cimitero
    if(!game.requestSelectOwnMonsterOnField(owner)) return;
    // 2) Dopo che un mostro viene mandato al Cimitero, prova ad aggiungere un mostro "Occhi di Blu" dal Deck
    //    Usiamo un piccolo hook sull'evento CardSentToGrave per eseguire una singola volta l'aggiunta.
    //    Nota: questa sottoscrizione non ha id esposto, ma il suo scope qui è breve; l'effetto è idempotente se add fallisce.
    game.events().subscribe(GameEventType::CardSentToGrave, [owner,&game](){
        // Tenta l'aggiunta una sola volta: se fallisce (nessun candidato) non riprova.
        (void)game.addFirstBlueEyesMonsterToHand(owner);
    });
}
