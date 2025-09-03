#pragma once
#include "../../../ICardEffect.h"
#include "../../../../Game.h"

// Saggio con Occhi di Blu — implementazione parziale
// Effetto considerato in questa patch: quando viene Evocato Normalmente:
// manda 1 mostro che controlli al Cimitero, poi aggiungi 1 mostro "Occhi di Blu" dal tuo Deck alla mano.
// Nota: l'attivazione dalla mano non è inclusa in questa patch.
class SageWithEyesOfBlueEffect : public ICardEffect {
public:
    void onEvent(GameEventType type, Game& game) override;
};
