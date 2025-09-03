#include "SageWithEyesOfBlueEffect.h"
#include <algorithm>

void SageWithEyesOfBlueEffect::onEvent(GameEventType type, Game& game){
    if(type != GameEventType::NormalSummon && type != GameEventType::MonsterSpecialSummoned) return;
    // Controlla se Saggio è appena entrato per il giocatore corrente
    int cur = game.getTurn().getCurrentPlayerIndex();
    const auto &mz = game.getMonsterZone();
    bool hasSage = std::any_of(mz.begin(), mz.end(), [](const Card& c){
        const std::string &name = c.getName();
        return name == std::string("Saggio con Occhi di Blu") || name == std::string("Saggio con gli Occhi Blu");
    });
    if(!hasSage) return;
    // Usa l'API dedicata per aggiungere un Drago di Livello 1 dal Deck
    if(game.hasPendingAddFromDeck()) return;
    (void)game.requestAddLevel1DragonFromDeck(cur);
}
