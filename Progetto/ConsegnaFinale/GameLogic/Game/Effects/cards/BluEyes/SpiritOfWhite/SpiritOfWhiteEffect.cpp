#include "SpiritOfWhiteEffect.h"
#include <algorithm>
#include "../../../../Game.h"

bool SpiritOfWhiteEffect::isSpiritOnField(Game& game) const{
    const auto &mz = game.getMonsterZone();
    return std::any_of(mz.begin(), mz.end(), [](const Card& c){ return c.getName() == std::string("Drago Spirito del Bianco"); });
}

void SpiritOfWhiteEffect::onEvent(GameEventType type, Game& game){
    if(type != GameEventType::NormalSummon && type != GameEventType::MonsterSpecialSummoned) return;
    // Attiva SOLO se l'ultimo mostro evocato è "Drago Spirito del Bianco" del giocatore corrente
    auto last = game.getLastSummonedMonster();
    if(!last.has_value()) return;
    int cur = game.getTurn().getCurrentPlayerIndex();
    if(!(last->ownerIdx == cur && last->name == std::string("Drago Spirito del Bianco"))) return;
    // Se gli effetti dei mostri del controllore sono annullati dal punto di vista dell'avversario, non procedere
    if(game.areOpponentMonsterEffectsNegatedFor(1 - cur)) return;
    // Richiedi la selezione di una M/T avversaria (includi Field Spell)
    if(game.hasPendingOpponentSpellTrapChoice()) return;
    (void)game.requestSelectOpponentSpellTrap(cur, /*includeFieldSpell*/true);
}
