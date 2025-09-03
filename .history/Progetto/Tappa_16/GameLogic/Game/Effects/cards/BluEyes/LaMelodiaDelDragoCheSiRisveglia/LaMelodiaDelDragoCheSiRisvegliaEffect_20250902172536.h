#pragma once
#include "../../../IActivationEffect.h"
#include "../../../../Game.h"

// Effetto di attivazione: "La Melodia del Drago che si Risveglia"
// Interattivo: il proprietario sceglie una carta in mano da scartare,
// poi sceglie dal Deck un Drago con ATK>=3000 e DEF<=2500 da aggiungere alla mano (riuso overlay DeckSend).
class LaMelodiaDelDragoCheSiRisvegliaEffect : public IActivationEffect {
public:
    bool canActivate(Game& game, int ownerIdx) const override;
    bool resolve(Game& game, int ownerIdx) override;
};
