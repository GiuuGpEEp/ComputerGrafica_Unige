#include "RivaliPredestinatiEffect.h"
#include <algorithm>

static bool controlsRequiredBoss(const Game& game, int ownerIdx) const{
    // Controlla se ownerIdx controlla BEWD o Mago Nero
    int cur = game.getTurn().getCurrentPlayerIndex();
    // Per semplicità, usiamo la zona mostri del proprietario a prescindere dal turno
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
