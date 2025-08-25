#pragma once
#include "../ICardEffect.h"
#include <string>

// Effetto: "Pietra Bianca della Leggenda"
// Se questa carta viene mandata al Cimitero: puoi aggiungere 1 "Drago Bianco Occhi Blu" dal tuo Deck alla tua mano.
class WhiteStoneLegendEffect : public ICardEffect {
public:
    void onEvent(GameEventType type, Game& game) override;
};
