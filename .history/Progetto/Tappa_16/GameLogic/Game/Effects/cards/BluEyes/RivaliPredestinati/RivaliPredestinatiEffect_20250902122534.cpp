#include "RivaliPredestinatiEffect.h"
#include <algorithm>

static bool controlsRequiredBoss(const Game& game, int ownerIdx){
    // Controlla se ownerIdx controlla BEWD o Mago Nero
    // Evita l'uso di getTurn() non const; seleziona la zona in base a ownerIdx rispetto al giocatore corrente.
    // Assumiamo che ownerIdx 0 sia il giocatore in basso e 1 quello in alto: mappiamo su zone esposte da Game
    // Game espone solo la zona del giocatore di turno e dell'avversario: deduciamo il mapping tramite indice turno volatile.
    // Workaround semplice: se ownerIdx==current, usa getMonsterZone, altrimenti getOpponentMonsterZone.
    int cur = const_cast<Game&>(game).getTurn().getCurrentPlayerIndex();
    const std::vector<Card>& zone = (ownerIdx == cur) ? game.getMonsterZone() : game.getOpponentMonsterZone();
    for(const auto& c : zone){
        const std::string& n = c.getName();
        if(n == "Drago Bianco Occhi Blu" || n == "Mago Nero"){ return true; }
    }
    return false;
}

bool RivaliPredestinatiEffect::canActivate(Game& game, int ownerIdx) const{
    return controlsRequiredBoss(game, ownerIdx);
}

bool RivaliPredestinatiEffect::resolve(Game& game, int ownerIdx){
    // Applica il flag di annullamento effetti mostri avversari fino a fine turno
    game.applyDestinedRivalsFor(ownerIdx);
    return true;
}
