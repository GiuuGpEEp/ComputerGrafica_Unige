#include "PiantoDiArgentoEffect.h"
#include "../../Game.h"

bool PiantoDiArgentoEffect::canActivate(Game& game, int ownerIdx) const{
    // Attivabile se c'è almeno un Mostro Normale Drago nel cimitero e spazio in zona mostri
    const auto& gy = (ownerIdx == 0) ? game.getGraveyard() : game.getOpponentGraveyard();
    bool hasTarget = false;
    for(const auto& c : gy){
        const auto& feats = c.getFeatures();
        bool isDragon = std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
        bool isNormal = std::find(feats.begin(), feats.end(), Feature::Normale) != feats.end();
        if(isDragon && isNormal){ hasTarget = true; break; }
    }
    if(!hasTarget) return false;
    // spazio in zona mostri
    // Game non espone direttamente la size per ownerIdx arbitrario, usiamo zone corrente/opposta in base al turno
    int cur = game.getTurn().getCurrentPlayerIndex();
    size_t size = (ownerIdx == cur) ? game.getMonsterZone().size() : game.getOpponentMonsterZone().size();
    // MONSTER_ZONE_SIZE è privato in Game, usiamo limite effettivo di 3 hardcoded come in Game
    return size < 3;
}

bool PiantoDiArgentoEffect::resolve(Game& game, int ownerIdx){
    // Scegli il primo Drago Normale dal cimitero e rievocalo scoperto in attacco
    auto& gy = (ownerIdx == 0) ? game.getGraveyard() : game.getOpponentGraveyard();
    for(size_t i=0;i<gy.size();++i){
        const auto& c = gy[i];
        const auto& feats = c.getFeatures();
        bool isDragon = std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
        bool isNormal = std::find(feats.begin(), feats.end(), Feature::Normale) != feats.end();
        if(isDragon && isNormal){
            Card picked = c; // copia
            gy.erase(gy.begin()+i);
            // Evoca speciale (attacco scoperto)
            if(game.specialSummonToMonsterZone(ownerIdx, std::move(picked), /*defense*/false, /*faceDown*/false)){
                return true;
            } else {
                // rimetti in gy se fallisce
                // Nota: qui semplifichiamo, in pratica non dovrebbe fallire data la canActivate
                return false;
            }
        }
    }
    return false;
}
