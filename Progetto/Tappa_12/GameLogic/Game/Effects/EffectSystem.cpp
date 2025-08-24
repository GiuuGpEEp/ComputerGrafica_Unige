#include "EffectSystem.h"
#include "ICardEffect.h"
#include "../Game.h"

void EffectSystem::registerEffectForCardName(const std::string& cardName, std::unique_ptr<ICardEffect> effect){
    registry[cardName] = std::move(effect);
}

void EffectSystem::dispatch(GameEventType type, Game& game){
    // Scansiona le zone mostri di entrambi i giocatori e notifica gli effetti per i nomi presenti
    for(int player = 0; player < 2; ++player){
        const auto& zone = (player == game.getTurn().getCurrentPlayerIndex())
            ? game.getMonsterZone()
            : game.getOpponentMonsterZone();
        for(const auto& c : zone){
            auto it = registry.find(c.getName());
            if(it != registry.end() && it->second){
                it->second->onEvent(type, game);
            }
        }
    }
}

void EffectSystem::clear(){
    registry.clear();
}
