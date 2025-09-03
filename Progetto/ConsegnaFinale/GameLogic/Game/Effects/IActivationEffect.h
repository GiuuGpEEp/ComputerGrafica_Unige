#pragma once
#include <string>

class Game;

// Interfaccia per effetti attivabili di Magie/Trappole (o carte con attivazione esplicita)
class IActivationEffect {
public:
    virtual ~IActivationEffect() = default;
    // Controlla se la carta può essere attivata nell'attuale stato del gioco da ownerIdx (0/1 assoluto)
    virtual bool canActivate(Game& game, int ownerIdx) const = 0;
    // Esegue la risoluzione dell'effetto. Ritorna true se l'effetto si è risolto con successo.
    virtual bool resolve(Game& game, int ownerIdx) = 0;
};
