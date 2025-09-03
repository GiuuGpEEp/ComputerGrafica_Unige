#pragma once
#include "../../../ICardEffect.h"

// Drago Spirito del Bianco
// Quando questa carta viene Evocata Normalmente o Specialmente: scegli 1 Magia/Trappola controllata dal tuo avversario; bandiscila.
class SpiritOfWhiteEffect : public ICardEffect {
public:
    void onEvent(GameEventType type, Game& game) override;
private:
    // Helper: true se il mostro con questo nome è sul campo del giocatore corrente
    bool isSpiritOnField(Game& game) const;
};
