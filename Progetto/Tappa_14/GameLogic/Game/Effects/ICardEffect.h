#pragma once
#include <memory>
#include <string>
#include "../../Events/EventDispatcher.h" // per GameEventType

class Game; // dichiarazione anticipata

// Interfaccia effetti minimale: per default senza stato; chiamata su eventi di gioco selezionati
class ICardEffect {
public:
    virtual ~ICardEffect() = default;
    // Gestisce un evento emesso. Gli effetti possono interrogare e modificare Game se necessario.
    virtual void onEvent(GameEventType type, Game& game) = 0;
};
