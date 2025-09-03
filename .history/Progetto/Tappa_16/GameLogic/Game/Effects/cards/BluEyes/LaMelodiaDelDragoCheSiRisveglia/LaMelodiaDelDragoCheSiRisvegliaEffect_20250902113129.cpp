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
    // 1) Scarta 1 carta dalla mano del proprietario (scegliamo l'ultima per semplicità)
    auto& hand = game.getHandOf(ownerIdx);
    if(hand.empty()) return false; // safety
    Card discarded = hand.back();
    hand.pop_back();
    // manda al cimitero del proprietario e traccia
    auto& gy = game.getGraveyardOf(ownerIdx);
    gy.push_back(std::move(discarded));
    game.lastSentToGrave = Game::LastSentToGrave{ gy.back().getName(), ownerIdx };
    game.events().emit(GameEventType::CardSentToGrave);
    game.events().emit(GameEventType::CardMoved);

    // 2) Aggiungi fino a 2 draghi target dal Deck alla mano
    Deck &deck = game.getDeckOf(ownerIdx);
    int added = 0;
    for(int i=0; i<2; ++i){
        auto found = deck.removeFirstByName("Drago Bianco Occhi Blu");
        if(!found.has_value()){
            // fallback generico: cerca qualunque Drago 3000/2500
            auto alt = deck.findFirst(isTargetDragon3000_2500);
            if(alt.has_value()){
                // remove by exact name to maintain single source of truth
                auto picked = deck.removeFirstByName(alt->getName());
                if(picked.has_value()){
                    game.mirrorExternalDeckRemoveByName(picked->getName(), &deck);
                    game.addToHand(ownerIdx, std::move(*picked));
                    ++added;
                }
            }
        } else {
            game.mirrorExternalDeckRemoveByName("Drago Bianco Occhi Blu", &deck);
            game.addToHand(ownerIdx, std::move(*found));
            ++added;
        }
    }
    (void)added; // not used further now
    return true;
}
