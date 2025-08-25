#include "ReliquarioDelDragoEffect.h"
#include "../../Game.h"

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
    // Manda 1 mostro Drago dal Deck al Cimitero (primo che troviamo)
    Deck& d = game.getDeckOf(ownerIdx);
    std::optional<Card> pick;
    // cerca primo Drago e rimuovi
    for(;;){
        auto found = d.findFirst([](const Card& c){
            const auto& feats = c.getFeatures();
            return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
        });
        if(found.has_value()){
            // removeFirstByName per semplicità (i nomi dei draghi nel deck sono univoci nelle nostre liste demo)
            pick = d.removeFirstByName(found->getName());
        }
        break;
    }
    if(!pick.has_value()) return false;
    // Inserisci nel Cimitero del proprietario
    auto& gy = (ownerIdx == 0) ? game.getGraveyard() : game.getOpponentGraveyard();
    gy.push_back(std::move(*pick));
    // Eventi: segnala invio al cimitero e movimento generico
    game.events().emit(GameEventType::CardSentToGrave);
    game.events().emit(GameEventType::CardMoved);
    return true;
}
