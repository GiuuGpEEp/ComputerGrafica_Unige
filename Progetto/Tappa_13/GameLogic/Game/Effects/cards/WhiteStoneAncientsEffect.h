#pragma once
#include "../ICardEffect.h"
#include <string>

class Game;

// Effetto: "Pietra Bianca degli Antichi"
// Se questa carta viene distrutta in battaglia o da un effetto: Evoca Specialmente 1 "Drago Bianco Occhi Blu" dal tuo Deck.
class WhiteStoneAncientsEffect : public ICardEffect {
public:
    void onEvent(GameEventType type, Game& game) override;
private:
    bool pendingThisTurn = false; // true se una "Pietra Bianca degli Antichi" è stata mandata al Cimitero in questo turno
    int ownerIdx = -1;            // proprietario della carta mandata al Cimitero
};
