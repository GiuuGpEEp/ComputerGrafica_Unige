#pragma once
#include "../../../IActivationEffect.h"
#include "../../../../Game.h"

// Attivazione dalla mano per "Saggio con Occhi di Blu"
// Costo: scartare questa carta dalla mano (gestito dal Game prima della resolve)
// Effetto: scegli 1 tuo mostro sul terreno; mandalo al Cimitero, poi Evoca Specialmente 1
// "Drago Bianco Occhi Blu" o 1 "Drago Spirito Occhi Blu" dalla tua mano, Deck o Cimitero.
class SageWithEyesOfBlueActivationEffect : public IActivationEffect {
public:
    bool canActivate(Game& game, int ownerIdx) const override;
    bool resolve(Game& game, int ownerIdx) override;
};
