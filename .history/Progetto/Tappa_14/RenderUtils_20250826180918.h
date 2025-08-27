#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "TextureManager/TextureManager.h"
#include "Config.h"

namespace RenderUtils {

// Se la carta ha una texture frontale definita, applicala e imposta il rect completo
inline void applyFrontTextureIfAny(Card& c, TextureManager& tm) {
    if(!c.originalTexturePath.empty()){
        sf::Texture &front = tm.getTexture(c.originalTexturePath);
        c.setTexture(front);
        sf::Vector2u ts = front.getSize();
        c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i((int)ts.x, (int)ts.y)));
    }
}

// Applica la texture del retro carta (CardBack) e imposta il rect completo
inline void applyBackTexture(Card& c, const std::unordered_map<AppConfig::TextureKey, sf::Texture*>& texMap) {
    sf::Texture &back = AppConfig::findTextureInMap(texMap, AppConfig::TextureKey::CardBack);
    c.setTexture(back);
    sf::Vector2u ts = back.getSize();
    c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i((int)ts.x, (int)ts.y)));
}

} // namespace RenderUtils
