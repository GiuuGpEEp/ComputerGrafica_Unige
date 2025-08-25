#include "EffectSystem.h"
#include "ICardEffect.h"
#include "IActivationEffect.h"
#include "../Game.h"

EffectSystem::~EffectSystem() = default;

void EffectSystem::registerEffectForCardName(const std::string& cardName, std::unique_ptr<ICardEffect> effect){
    registry[cardName] = std::move(effect);
}

void EffectSystem::dispatch(GameEventType type, Game& game){
    if(dispatching) return; // evita ricorsione/ri-entrancy semplice
    dispatching = true;
    // Versione minimale: notifica TUTTI gli effetti registrati.
    // Ogni effetto decide internamente se reagire in base all'evento e allo stato del gioco.
    for(auto &kv : registry){
        if(kv.second){ kv.second->onEvent(type, game); }
    }
    dispatching = false;
}

void EffectSystem::clear(){
    registry.clear();
    activationRegistry.clear();
}

void EffectSystem::registerActivationForCardName(const std::string& cardName, std::unique_ptr<IActivationEffect> effect){
    activationRegistry[cardName] = std::move(effect);
}

IActivationEffect* EffectSystem::getActivationFor(const std::string& cardName) const{
    auto it = activationRegistry.find(cardName);
    if(it == activationRegistry.end()) return nullptr;
    return it->second.get();
}
