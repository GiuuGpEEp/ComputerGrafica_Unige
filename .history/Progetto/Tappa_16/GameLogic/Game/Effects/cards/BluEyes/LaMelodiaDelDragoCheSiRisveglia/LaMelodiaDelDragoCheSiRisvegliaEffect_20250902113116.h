#pragma once
#include "../../../IActivationEffect.h"
#include "../../../../Game.h"

// Effetto di attivazione: "La Melodia del Drago che si Risveglia"
// Semplificazione: scarta 1 carta dalla mano del proprietario (l'ultima),
// poi aggiunge fino a 2 Draghi con ATK>=3000 e DEF<=2500 dal Deck alla mano (auto-scelta).
class LaMelodiaDelDragoCheSiRisvegliaEffect : public IActivationEffect {
public:
    bool canActivate(Game& game, int ownerIdx) const override;
    bool resolve(Game& game, int ownerIdx) override;
};
