#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include "../../Events/EventDispatcher.h" // GameEventType
#include <optional>

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

    // Chain di attivazioni: richieste di attivazione (es. Magie/Trappole attivate)
    struct ActivationRequest { std::string cardName; int ownerIdx; };
    std::vector<ActivationRequest> activationChain; // LIFO stack

public:
    // Effect chain API: aggiunge una richiesta di attivazione in cima alla catena
    void pushActivationRequest(const std::string& cardName, int ownerIdx);
    // Risolvi la richiesta in cima alla catena (LIFO). Ritorna false se non esiste effetto o fallisce.
    bool resolveTopActivation(Game& game);
    // Risolvi tutte le richieste nella chain fino a svuotarla
    void resolveAllActivations(Game& game);
    bool hasPendingActivations() const { return !activationChain.empty(); }

    // Osserva l'attivazione in cima senza rimuoverla. Restituisce std::nullopt se vuota.
    std::optional<std::pair<std::string,int>> peekTopActivation() const;

public:
    // Accesso read-only: trova un effetto di attivazione per nome carta
    IActivationEffect* getActivationFor(const std::string& cardName) const;
};
