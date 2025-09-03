#include "SageWithEyesOfBlueEffect.h"
#include <algorithm>

void SageWithEyesOfBlueEffect::onEvent(GameEventType type, Game& game){
    if(type != GameEventType::NormalSummon && type != GameEventType::MonsterSpecialSummoned) return;
    // Controlla se Saggio è appena entrato per il giocatore corrente
    int cur = game.getTurn().getCurrentPlayerIndex();
    const auto &mz = game.getMonsterZone();
    bool hasSage = std::any_of(mz.begin(), mz.end(), [](const Card& c){ return c.getName() == std::string("Saggio con Occhi di Blu"); });
    if(!hasSage) return;
    // Richiedi una scelta di aggiunta: per ora riusa l'evento generale DeckAddChoiceRequested e filtra lato UI.
    // Evita sovrapposizioni con Melodia o altre pending
    if(game.hasPendingAddFromDeck()) return;
    // Requisiti: esista almeno un Drago di Livello 1 nel Deck del giocatore
    const Deck &d = game.getDeckOf(cur);
    auto candidates = d.collectWhere([](const Card& c){
        if(c.getType() != Type::Monster) return false;
        auto lv = c.getLevelOrRank(); if(!lv.has_value() || *lv != 1) return false;
        const auto &feats = c.getFeatures();
        return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
    });
    if(candidates.empty()) return;
    // Hack semplice: usa l'infrastruttura di add standard e delega al filtro AppHandlers in futuro.
    (void)game.requestAddFromDeck(cur);
}
