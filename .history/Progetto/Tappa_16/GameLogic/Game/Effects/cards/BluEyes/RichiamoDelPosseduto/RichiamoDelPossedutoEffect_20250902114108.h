#pragma once
#include "../../../IActivationEffect.h"
#include "../../../../Game.h"

// Effetto di attivazione: "Richiamo del Posseduto" (versione semplificata)
// Attiva: Evoca Specialmente 1 mostro dal tuo Cimitero in Posizione di Attacco.
// Nota: la parte di distruzione collegata alla Trappola è omessa per ora.
class RichiamoDelPossedutoEffect : public IActivationEffect {
public:
    bool canActivate(Game& game, int ownerIdx) const override;
    bool resolve(Game& game, int ownerIdx) override;
};
