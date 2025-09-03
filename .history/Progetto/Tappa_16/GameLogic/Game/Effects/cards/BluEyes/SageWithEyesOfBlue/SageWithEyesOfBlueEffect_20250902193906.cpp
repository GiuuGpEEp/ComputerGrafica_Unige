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
    // Quando questa carta viene Evocata: aggiungi 1 mostro Drago di Livello 1 dalla mano, Deck o Cimitero alla mano.
    // Implementiamo l'add dal Deck (UI overlay). Mano/GY add automatiche richiederebbero un overlay dedicato; per ora Deck.
    (void)game.requestAddLevel1DragonToHand(owner);
}
