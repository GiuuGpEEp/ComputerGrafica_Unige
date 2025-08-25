#include "EffectSystem.h"
#include "ICardEffect.h"
#include "../Game.h"

void EffectSystem::registerEffectForCardName(const std::string& cardName, std::unique_ptr<ICardEffect> effect){
    registry[cardName] = std::move(effect);
}

void EffectSystem::dispatch(GameEventType type, Game& game){
    if(dispatching) return; // evita ricorsione/ri-entrancy semplice
    dispatching = true;
    // Cattura uno snapshot dei nomi delle carte attualmente sul campo (entrambi i giocatori)
    std::vector<std::string> names;
    names.reserve(8);
    for(int player = 0; player < 2; ++player){
        const auto& zone = (player == game.getTurn().getCurrentPlayerIndex())
            ? game.getMonsterZone()
            : game.getOpponentMonsterZone();
        for(const auto& c : zone){ names.push_back(c.getName()); }
    }
    // Esegui gli effetti in base ai nomi snapshot
    for(const auto& name : names){
        auto it = registry.find(name);
        if(it != registry.end() && it->second){
            it->second->onEvent(type, game);
        }
    }
    dispatching = false;
}

void EffectSystem::clear(){
    registry.clear();
}
