#pragma once
#include "../../../ICardEffect.h"
#include "../../../../Game.h"
#include <string>

class Game;

// Effetto: "Pietra Bianca degli Antichi"
// Se questa carta viene mandata al cimitero in QUESTO turno (in qualsiasi modo),
// il suo proprietario può Evocare Specialmente 1 "Drago Bianco Occhi Blu" dal proprio Deck
// durante l'End Phase dello stesso turno.
class WhiteStoneAncientsEffect : public ICardEffect {
public:
    void onEvent(GameEventType type, Game& game) override;
private:
    bool pendingThisTurn = false; // true se una "Pietra Bianca degli Antichi" è stata mandata al Cimitero in questo turno
    int ownerIdx = -1;            // proprietario della carta mandata al Cimitero
};
