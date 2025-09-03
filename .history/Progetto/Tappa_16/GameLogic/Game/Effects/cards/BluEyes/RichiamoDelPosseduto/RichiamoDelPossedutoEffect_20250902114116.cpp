#include "RichiamoDelPossedutoEffect.h"
#include <algorithm>

bool RichiamoDelPossedutoEffect::canActivate(Game& game, int ownerIdx) const{
    // Serve almeno un mostro nel Cimitero e spazio in zona mostri (3-slot)
    const auto& gy = game.getGraveyardOf(ownerIdx);
    bool hasMonster = std::any_of(gy.begin(), gy.end(), [](const Card& c){ return c.getType()==Type::Monster; });
    if(!hasMonster) return false;
    int cur = game.getTurn().getCurrentPlayerIndex();
    size_t size = (ownerIdx == cur) ? game.getMonsterZone().size() : game.getOpponentMonsterZone().size();
    return size < 3;
}

bool RichiamoDelPossedutoEffect::resolve(Game& game, int ownerIdx){
    // Scegli il primo mostro in GY e rievocalo in Attacco scoperto
    auto& gy = game.getGraveyardOf(ownerIdx);
    for(size_t i=0;i<gy.size();++i){
        if(gy[i].getType()==Type::Monster){
            Card picked = gy[i];
            gy.erase(gy.begin()+i);
            return game.specialSummonToMonsterZone(ownerIdx, std::move(picked), /*defense*/false, /*faceDown*/false);
        }
    }
    return false;
}
