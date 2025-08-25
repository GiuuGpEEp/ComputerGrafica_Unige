#pragma once
#include "../IActivationEffect.h"

class ReliquarioDelDragoEffect : public IActivationEffect {
public:
    bool canActivate(Game& game, int ownerIdx) const override;
    bool resolve(Game& game, int ownerIdx) override;
};
