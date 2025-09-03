#pragma once
#include "../../../ICardEffect.h"
#include "../../../EffectSystem.h"
#include "../../../../Game.h"
#include "../../../../../Player/Player.h"
#include <string>

// Effetto: "Pietra Bianca della Leggenda"
// Se questa carta viene mandata al Cimitero: puoi aggiungere 1 "Drago Bianco Occhi Blu" dal tuo Deck alla tua mano.
class TextureManager; // forward

class WhiteStoneLegendEffect : public ICardEffect {
public:
    explicit WhiteStoneLegendEffect(TextureManager& tm);
    void onEvent(GameEventType type, Game& game) override;
private:
    TextureManager* textureManager = nullptr;
};
