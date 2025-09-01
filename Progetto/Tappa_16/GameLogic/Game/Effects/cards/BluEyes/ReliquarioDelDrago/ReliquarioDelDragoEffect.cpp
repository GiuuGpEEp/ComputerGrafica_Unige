#include "ReliquarioDelDragoEffect.h"


bool ReliquarioDelDragoEffect::canActivate(Game& game, int ownerIdx) const{
    // Attivabile se nel Deck del giocatore esiste almeno 1 mostro Drago
    const Deck& d = game.getDeckOf(ownerIdx);
    auto hasDragon = d.findFirst([](const Card& c){
        const auto& feats = c.getFeatures();
        return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
    });
    return hasDragon.has_value();
}

bool ReliquarioDelDragoEffect::resolve(Game& game, int ownerIdx){
    // Richiedi alla UI di scegliere quale Drago inviare dal Deck al Cimitero
    // L'UI filtrerà i draghi dal Deck del proprietario e, alla conferma, rimuoverà quello scelto
    game.requestSendFromDeck(ownerIdx);
    return true; // l'attivazione procede; l'invio al GY avverrà on-choice lato UI
}
