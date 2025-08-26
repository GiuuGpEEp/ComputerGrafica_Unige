#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include "../../Events/EventDispatcher.h" // GameEventType

// Forward declarations per evitare dipendenze circolari
class ICardEffect;
class IActivationEffect;
class Game;

// Sistema effetti minimale: registra effetti per nome carta e li notifica sugli eventi
class EffectSystem {
public:
    ~EffectSystem();
    // Registra un effetto associato al nome carta (sovrascrive eventuali precedenti)
    void registerEffectForCardName(const std::string& cardName, std::unique_ptr<ICardEffect> effect);
    // Registra un effetto di attivazione associato al nome carta
    void registerActivationForCardName(const std::string& cardName, std::unique_ptr<IActivationEffect> effect);

    // Notifica tutti gli effetti delle carte attualmente sul campo (entrambi i giocatori)
    void dispatch(GameEventType type, Game& game);

    // Svuota il registro
    void clear();

private:
    // Registro per nome carta
    std::unordered_map<std::string, std::unique_ptr<ICardEffect>> registry;
    std::unordered_map<std::string, std::unique_ptr<IActivationEffect>> activationRegistry;
    // Semplice guard per prevenire re-entrancy durante il dispatch (effetti che scatenano altri eventi)
    bool dispatching = false;

public:
    // Accesso read-only: trova un effetto di attivazione per nome carta
    IActivationEffect* getActivationFor(const std::string& cardName) const;
};
