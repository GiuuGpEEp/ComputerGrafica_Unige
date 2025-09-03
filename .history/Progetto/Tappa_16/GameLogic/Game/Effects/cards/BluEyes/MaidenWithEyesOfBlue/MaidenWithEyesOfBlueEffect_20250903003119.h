#pragma once
#include "../../../ICardEffect.h"
#include "../../../EffectSystem.h"
#include "../../../../Game.h"

// "Fanciulla con gli Occhi di Blu" — Quando questa carta viene scelta come bersaglio di un attacco:
// Puoi Evocare Specialmente 1 "Drago Bianco Occhi Blu" o 1 "Drago Spirito Occhi Blu" dalla tua mano, Deck o Cimitero.
// Nota: questa implementazione si attiva su AttackDeclared e usa il contesto dell'attacco esposto da Game.
class MaidenWithEyesOfBlueEffect : public ICardEffect {
public:
    void onEvent(GameEventType type, Game& game) override;
};
