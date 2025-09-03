#pragma once
#include "../../../ICardEffect.h"
#include "../../../EffectSystem.h"
#include "../../../../Game.h"

// Effetto on-summon: Quando questa carta viene Evocata Normalmente o Specialmente: puoi aggiungere 1 mostro Drago di Livello 1 dalla tua mano, Deck o Cimitero alla tua mano.
class SageWithEyesOfBlueEffect : public ICardEffect {
public:
    void onEvent(GameEventType type, Game& game) override;
};
