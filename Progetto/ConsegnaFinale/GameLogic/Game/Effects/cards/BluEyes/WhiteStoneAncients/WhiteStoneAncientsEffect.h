#pragma once
#include "../../../ICardEffect.h"
#include "../../../../Game.h"
#include <string>
#include <array>

class Game;

// Effetto: "Pietra Bianca degli Antichi"
// Se questa carta viene mandata al cimitero in QUESTO turno (in qualsiasi modo),
// il suo proprietario può Evocare Specialmente 1 "Drago Bianco Occhi Blu" dal proprio Deck
// durante l'End Phase dello stesso turno.
class WhiteStoneAncientsEffect : public ICardEffect {
public:
    void onEvent(GameEventType type, Game& game) override;
private:
    // Per supportare attivazioni simultanee (entrambi i giocatori nello stesso turno),
    // tracciamo uno stato per giocatore.
    std::array<bool, 2> pendingFor { false, false }; // pending per ciascun giocatore [0]=P1, [1]=P2
};
