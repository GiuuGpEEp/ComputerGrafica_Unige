#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <functional>
#include <vector>
#include <optional>
#include <string>
#include "TextureManager/TextureManager.h"
#include "Config.h"

namespace RenderUtils {

// Costanti di scala usate nel layout
inline constexpr float FieldScale = 0.75f;       // carte mostri in piedi nello slot
inline constexpr float STScale = 0.70f;          // carte Magia/Trappola nello slot
inline constexpr float FieldSpellScale = 0.78f;  // carta Terreno nello slot

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
// (helper rimosso per evitare dipendenze su Game/Field/Type in header)

// Misc: mix di due colori (media canali, alpha del primo)
inline sf::Color mixColors(sf::Color a, sf::Color b) {
    return sf::Color(
        static_cast<uint8_t>(std::min(255, (int(a.r) + int(b.r)) / 2)),
        static_cast<uint8_t>(std::min(255, (int(a.g) + int(b.g)) / 2)),
        static_cast<uint8_t>(std::min(255, (int(a.b) + int(b.b)) / 2)),
        a.a
    );
}

// Disegna il pallino d'attacco in alto a sinistra della carta
inline void drawAttackDot(sf::RenderWindow& window, const Card& c,
                          float radius = 9.f,
                          sf::Color fill = sf::Color(220,60,60,230),
                          sf::Color outline = sf::Color(0,0,0,200),
                          float outlineThickness = 2.f,
                          sf::Vector2f offset = sf::Vector2f(6.f, 6.f)) {
    sf::FloatRect gb = c.getGlobalBounds();
    sf::CircleShape dot(radius);
    dot.setFillColor(fill);
    dot.setOutlineColor(outline);
    dot.setOutlineThickness(outlineThickness);
    dot.setPosition(sf::Vector2f(gb.position.x + offset.x, gb.position.y + offset.y));
    window.draw(dot);
}

// =====================
// Batch draw helpers
// =====================
// Disegna una riga di carte (M/T) applicando il retro dove necessario
inline void drawSpellTrapRow(
    sf::RenderWindow& window,
    const std::vector<Card>& cards,
    const std::function<bool(size_t)>& isFaceDownAtIndex,
    const std::unordered_map<AppConfig::TextureKey, sf::Texture*>& texMap
) {
    for (size_t i = 0; i < cards.size(); ++i) {
        Card c = cards[i];
        if (isFaceDownAtIndex && isFaceDownAtIndex(i)) {
            RenderUtils::applyBackTexture(c, texMap);
        }
        c.draw(window);
    }
}

// Disegna opzionalmente una carta (es. Field Spell) se presente
inline void drawFieldSpellOpt(sf::RenderWindow& window, std::optional<Card>& opt) {
    if (opt.has_value()) {
        opt.value().draw(window);
    }
}

// Disegna la top card in uno slot (es. Cimitero), scalata e centrata
inline void drawTopCardAt(
    sf::RenderWindow& window,
    const std::optional<Card>& topCard,
    const sf::Vector2f& slotPos,
    const sf::Vector2f& slotSize,
    TextureManager& textureManager,
    float scale = RenderUtils::FieldScale
) {
    if (!topCard.has_value()) return;
    Card copy = topCard.value();
    RenderUtils::applyFrontTextureIfAny(copy, textureManager);
    RenderUtils::placeCentered(copy, slotPos, slotSize, scale, scale);
    copy.draw(window);
}

// =====================
// Overlay helpers
// =====================
// Disegna l'overlay di Game Over con pannello centrale, titolo e un hint
inline void drawGameOverOverlay(
    sf::RenderWindow& window,
    const sf::Font& font,
    const sf::Vector2u& windowSize,
    const std::string& titleStr,
    const std::string& hintStr = "Premi SPACE per tornare alla Home"
) {
    // Sfondo scuro
    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
    overlay.setFillColor(sf::Color(0,0,0,160));
    window.draw(overlay);

    // Pannello centrale
    sf::Vector2f panelSize(720.f, 320.f);
    sf::Vector2f center(windowSize.x/2.f, windowSize.y/2.f);
    sf::Vector2f pos(center.x - panelSize.x/2.f, center.y - panelSize.y/2.f);
    sf::RectangleShape panel(panelSize);
    panel.setPosition(pos);
    panel.setFillColor(sf::Color(25,25,25,235));
    panel.setOutlineColor(sf::Color(255,200,90));
    panel.setOutlineThickness(3.f);
    window.draw(panel);

    // Titolo
    sf::Text title(font, titleStr, 54);
    title.setFillColor(sf::Color(255,230,140));
    title.setOutlineColor(sf::Color(0,0,0,200));
    title.setOutlineThickness(3.f);
    sf::FloatRect tB = title.getLocalBounds();
    title.setOrigin(sf::Vector2f(tB.position.x + tB.size.x/2.f, tB.position.y + tB.size.y/2.f));
    title.setPosition(sf::Vector2f(center.x, pos.y + 110.f));
    window.draw(title);

    // Hint
    sf::Text hint(font, hintStr, 26);
    hint.setFillColor(sf::Color(240,240,240,240));
    hint.setOutlineColor(sf::Color(0,0,0,200));
    hint.setOutlineThickness(2.f);
    sf::FloatRect hB = hint.getLocalBounds();
    hint.setOrigin(sf::Vector2f(hB.position.x + hB.size.x/2.f, hB.position.y + hB.size.y/2.f));
    hint.setPosition(sf::Vector2f(center.x, pos.y + panelSize.y - 70.f));
    window.draw(hint);
}

} // namespace RenderUtils
