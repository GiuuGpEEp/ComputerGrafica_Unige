#pragma once
#include "../../../IActivationEffect.h"
#include "../../../../Game.h"

// "Rivali Predestinati": se controlli BEWD o Mago Nero, annulla gli effetti dei mostri scoperti dell'avversario fino alla fine del turno.
class RivaliPredestinatiEffect : public IActivationEffect {
public:
    bool canActivate(Game& game, int ownerIdx) const override;
    bool resolve(Game& game, int ownerIdx) override;
    const static bool controlsRequiredBoss(const Game& game, int ownerIdx);
};
