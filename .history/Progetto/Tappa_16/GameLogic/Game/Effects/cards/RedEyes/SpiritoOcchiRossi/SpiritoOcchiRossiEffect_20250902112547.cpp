#include "SpiritoOcchiRossiEffect.h"
#include <algorithm>
#include "../../../EffectSystem.h"
#include "../../../ICardEffect.h"

// Helper: true se la carta appartiene all'archetipo "Occhi Rossi"
static bool isRedEyes(const Card& c){
    const std::string& n = c.getName();
    return n.find("Occhi Rossi") != std::string::npos || n.find("Occhi-Rossi") != std::string::npos;
}

bool SpiritoOcchiRossiEffect::canActivate(Game& game, int ownerIdx) const{
    // Attivabile se nel Cimitero del proprietario esiste almeno un mostro "Occhi Rossi"
    const auto& gy = game.getGraveyardOf(ownerIdx);
    auto it = std::find_if(gy.begin(), gy.end(), [](const Card& c){ return isRedEyes(c) && c.getType()==Type::Monster; });
    if(it == gy.end()) return false;
    // Serve spazio libero in zona mostri del proprietario
    int cur = game.getTurn().getCurrentPlayerIndex();
    size_t size = (ownerIdx == cur) ? game.getMonsterZone().size() : game.getOpponentMonsterZone().size();
    return size < 3; // coerente con limite v0
}

bool SpiritoOcchiRossiEffect::resolve(Game& game, int ownerIdx){
    auto& gy = game.getGraveyardOf(ownerIdx);
    for(size_t i=0;i<gy.size();++i){
        if(isRedEyes(gy[i]) && gy[i].getType()==Type::Monster){
            Card picked = gy[i];
            gy.erase(gy.begin()+i);
            return game.specialSummonToMonsterZone(ownerIdx, std::move(picked), /*defense*/false, /*faceDown*/false);
        }
    }
    return false;
}
