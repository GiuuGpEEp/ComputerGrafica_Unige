#pragma once
#include "../../../IActivationEffect.h"

// Effetto di attivazione: "Spirito Occhi Rossi"
// Testo: "Scegli come bersaglio 1 mostro \"Occhi Rossi\" nel tuo Cimitero; Evocalo Specialmente."
// Implementazione semplificata: non apre UI di selezione, sceglie il primo bersaglio valido trovato.
class SpiritoOcchiRossiEffect : public IActivationEffect {
public:
    bool canActivate(Game& game, int ownerIdx) const override;
    bool resolve(Game& game, int ownerIdx) override;
};
