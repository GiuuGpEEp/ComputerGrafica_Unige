#include "MaidenWithEyesOfBlueEffect.h"
#include <optional>

void MaidenWithEyesOfBlueEffect::onEvent(GameEventType type, Game& game){
    if(type != GameEventType::AttackDeclared) return;
    auto ctxOpt = game.getLastDeclaredAttack();
    if(!ctxOpt.has_value()) return;
    const auto &ctx = *ctxOpt;
    // Deve esserci un bersaglio dichiarato
    if(!ctx.targetIndex.has_value()) return;
    const int defender = ctx.defenderOwner;
    const size_t tIdx = *ctx.targetIndex;
    const auto &oppMZ = (defender == game.getTurn().getCurrentPlayerIndex())
        ? game.getOpponentMonsterZone() // Se per qualche motivo i riferimenti cambiano, usiamo API generiche
        : game.getMonsterZone();
    // Più robusto: interroga direttamente la zona mostri del difensore
    // Nota: Game non espone direttamente getMonsterZoneOf(player), quindi accediamo via indexing interno
    // qui assumiamo che ctx.defenderOwner sia l'indice assoluto 0/1 (coerente con Game::declareAttack)
    // Usiamo un piccolo switch per leggere il nome
    std::string targetName;
    if(defender >= 0 && defender <= 1){
        const auto &zoneRef = (defender == game.getTurn().getCurrentPlayerIndex())
            ? game.getMonsterZone() // current() == defender
            : game.getOpponentMonsterZone();
        if(tIdx >= zoneRef.size()) return;
        targetName = zoneRef[tIdx].getName();
    } else {
        return;
    }

    // Controlla negazione effetti: se i mostri del difensore hanno effetti annullati per questo turno, non procedere
    if(game.areOpponentMonsterEffectsNegatedFor(1 - defender)){
        // Per il giocatore X, "areOpponentMonsterEffectsNegatedFor(X) == true" significa che gli EFFECT dei mostri del suo avversario sono annullati.
        // Qui dobbiamo verificare se gli effetti del difensore sono annullati; la proprietà esposta è relativa al punto di vista di chi interroga.
        // Se gli effetti del difensore sono annullati, dovrebbe risultare vero quando query-iamo dal punto di vista dell'attaccante (1 - defender).
        return;
    }

    // Match robusto sul nome della Fanciulla (varianti con/without "gli")
    if(!(targetName == std::string("Fanciulla con gli Occhi di Blu") || targetName == std::string("Fanciulla con Occhi di Blu"))){
        return;
    }

    // Evita re-entrancy: se già c'è una scelta BE/Spirit in corso, non sovrapporre
    if(game.hasPendingBlueEyesSSChoice()) return;

    // Richiedi la scelta al difensore (owner della Fanciulla bersagliata)
    (void)game.requestBlueEyesOrSpiritChoice(defender);
}
