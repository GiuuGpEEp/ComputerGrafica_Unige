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

// Posiziona e centra la carta in uno slot, senza rotazione
inline void placeCentered(Card& c, const sf::Vector2f& slotPos, const sf::Vector2f& slotSize,
                          float scaleX = 1.f, float scaleY = 1.f) {
    sf::Vector2f size(slotSize.x * scaleX, slotSize.y * scaleY);
    c.setSize(size);
    c.setPosition(slotPos + sf::Vector2f((slotSize.x - size.x) / 2.f, (slotSize.y - size.y) / 2.f));
}

// Layout difesa/orizzontale: ruota di 90° e centra nello slot con spessore/lunghezza personalizzabili
inline void placeRotatedDefense(Card& c, const sf::Vector2f& slotPos, const sf::Vector2f& slotSize,
                                float thicknessFactor = 0.60f, float lengthFactor = 0.95f) {
    // Dimensione target prima della rotazione: (spessore x lunghezza)
    sf::Vector2f rotatedSize(slotSize.y * thicknessFactor, slotSize.x * lengthFactor);
    c.setSize(rotatedSize);
    c.centerOrigin();
    c.setRotation(90.f);
    // Centro dello slot
    sf::Vector2f slotCenter(slotPos.x + slotSize.x / 2.f, slotPos.y + slotSize.y / 2.f);
    c.setPosition(slotCenter);
}

// Resetta la carta in piedi (nessuna rotazione/origine al top-left)
inline void setUpright(Card& c) {
    c.resetOrigin();
    c.setRotation(0.f);
}

// Ruota di 180° e posiziona al centro del rettangolo definito da top-left e size
inline void rotate180Centered(Card& c, const sf::Vector2f& topLeftPos, const sf::Vector2f& size) {
    c.setSize(size);
    c.centerOrigin();
    c.setRotation(180.f);
    sf::Vector2f center(topLeftPos.x + size.x / 2.f, topLeftPos.y + size.y / 2.f);
    c.setPosition(center);
}

} // namespace RenderUtils
