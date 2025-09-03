#include "SageWithEyesOfBlueEffect.h"
#include <algorithm>

void SageWithEyesOfBlueEffect::onEvent(GameEventType type, Game& game){
    if(type != GameEventType::NormalSummon && type != GameEventType::MonsterSpecialSummoned) return;
    // Attiva SOLO se l'ultimo mostro evocato è "Saggio con (gli) Occhi Blu" del giocatore corrente
    auto last = game.getLastSummonedMonster();
    if(!last.has_value()) return;
    int cur = game.getTurn().getCurrentPlayerIndex();
    const bool isSage = (last->name == std::string("Saggio con Occhi di Blu") || last->name == std::string("Saggio con gli Occhi Blu"));
    if(!(last->ownerIdx == cur && isSage)) return;
    // Se gli effetti dei mostri del controllore sono annullati in questo turno, non procedere
    if(game.areOpponentMonsterEffectsNegatedFor(1 - cur)) return;
    // Usa l'API dedicata per aggiungere un Drago di Livello 1 dal Deck
    if(game.hasPendingAddFromDeck()) return;
    (void)game.requestAddLevel1DragonFromDeck(cur);
}
