#include "LaMelodiaDelDragoCheSiRisvegliaEffect.h"
#include <algorithm>

static bool isTargetDragon3000_2500(const Card& c){
    auto val = c.getValues();
    if(!val.has_value()) return false;
    int atk = val->first;
    int def = val->second;
    if(atk < 3000 || def > 2500) return false;
    const auto& feats = c.getFeatures();
    return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
}

bool LaMelodiaDelDragoCheSiRisvegliaEffect::canActivate(Game& game, int ownerIdx) const{
    // Richiede almeno 1 carta in mano da scartare e almeno 1 bersaglio valido nel Deck del proprietario
    auto& hand = game.getHandOf(ownerIdx);
    if(hand.empty()) return false;
    const Deck& deck = game.getDeckOf(ownerIdx);
    auto any = deck.findFirst(isTargetDragon3000_2500);
    return any.has_value();
}

bool LaMelodiaDelDragoCheSiRisvegliaEffect::resolve(Game& game, int ownerIdx){
    // Flusso interattivo:
    // 1) Richiedi scelta carta da scartare dalla mano
    if(!game.requestHandDiscard(ownerIdx)) return false;
    
    // Per consentire l'auto avvio impostiamo il flag
    game.setNextAddFromDeckAfterDiscard(ownerIdx);
    return true; // l'effetto prosegue tramite eventi/handlers
}
