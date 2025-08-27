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
// Disegna una singola carta Mostro nello slot specificato applicando rotazioni/texture di retro dove richiesto
// sideTop=false => lato basso (P1), sideTop=true => lato alto (P2)
inline void drawMonsterCard(
    sf::RenderWindow& window,
    Card cardCopy,
    bool sideTop,
    const sf::Vector2f& slotPos,
    const sf::Vector2f& slotSize,
    bool isDefense,
    bool isFaceDown,
    bool isSelectedAttacker,
    bool showAttackDot,
    const std::unordered_map<AppConfig::TextureKey, sf::Texture*>& texMap
) {
    sf::Color color = sf::Color::White;
    if (isSelectedAttacker) {
        color = sf::Color(255,240,160);
    }
    if (isFaceDown) {
        RenderUtils::applyBackTexture(cardCopy, texMap);
    }
    if (isDefense || isFaceDown) {
        RenderUtils::placeRotatedDefense(cardCopy, slotPos, slotSize);
    } else {
        if (!sideTop) {
            // P1 in piedi
            RenderUtils::setUpright(cardCopy);
        } else {
            // P2 in piedi capovolta 180°
            float s = RenderUtils::FieldScale;
            sf::Vector2f scaledSize(slotSize.x * s, slotSize.y * s);
            sf::Vector2f topLeft = slotPos + sf::Vector2f((slotSize.x - scaledSize.x) / 2.f, (slotSize.y - scaledSize.y) / 2.f);
            RenderUtils::rotate180Centered(cardCopy, topLeft, scaledSize);
        }
    }
    if (isDefense) {
        color = RenderUtils::mixColors(color, sf::Color(150,200,255,color.a));
    }
    cardCopy.draw(window, color);
    if (showAttackDot) {
        RenderUtils::drawAttackDot(window, cardCopy);
    }
}

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

// Overlay conferma ritorno alla Home (con hover dei pulsanti)
inline void drawConfirmReturnHomeOverlay(
    sf::RenderWindow& window,
    const sf::Font& font,
    const sf::Vector2u& windowSize,
    const sf::Vector2i& mousePos
) {
    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
    overlay.setFillColor(sf::Color(0,0,0,140));
    window.draw(overlay);

    sf::Vector2f popupSize(600.f,300.f);
    sf::Vector2f center(windowSize.x/2.f, windowSize.y/2.f);
    sf::Vector2f popupPos(center.x - popupSize.x/2.f, center.y - popupSize.y/2.f);
    sf::RectangleShape panel(popupSize);
    panel.setPosition(popupPos);
    panel.setFillColor(sf::Color(25,25,25,235));
    panel.setOutlineColor(sf::Color(255,200,90));
    panel.setOutlineThickness(3.f);
    window.draw(panel);

    sf::Text title(font, "Torna alla Home?", 48);
    title.setFillColor(sf::Color(255,230,140));
    title.setOutlineColor(sf::Color(0,0,0,200));
    title.setOutlineThickness(3.f);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(sf::Vector2f(titleBounds.size.x/2.f, titleBounds.size.y/2.f));
    title.setPosition(sf::Vector2f(center.x, popupPos.y + 70.f));
    window.draw(title);

    // Bottoni grafici
    sf::Vector2f btnSize(160.f,60.f);
    float gap = 80.f;
    float totalBtnWidth = btnSize.x*2 + gap;
    float startX = center.x - totalBtnWidth/2.f;
    float bottomMargin = 60.f;
    float buttonY = popupPos.y + popupSize.y - btnSize.y - bottomMargin;
    sf::RectangleShape yesButton(btnSize);
    yesButton.setPosition(sf::Vector2f(startX, buttonY));
    sf::RectangleShape noButton(btnSize);
    noButton.setPosition(sf::Vector2f(startX + btnSize.x + gap, buttonY));

    auto lighten = [](sf::Color c){
        return sf::Color(
            static_cast<uint8_t>(std::min(255, int(c.r*1.25f))),
            static_cast<uint8_t>(std::min(255, int(c.g*1.25f))),
            static_cast<uint8_t>(std::min(255, int(c.b*1.25f))),
            c.a
        );
    };

    sf::Vector2f mouseF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    bool hoverYes = yesButton.getGlobalBounds().contains(mouseF);
    bool hoverNo  = noButton.getGlobalBounds().contains(mouseF);
    auto baseYes = sf::Color(60,160,60);
    auto baseNo  = sf::Color(170,60,60);
    yesButton.setFillColor(hoverYes ? lighten(baseYes) : baseYes);
    noButton.setFillColor(hoverNo ? lighten(baseNo) : baseNo);
    yesButton.setOutlineThickness(hoverYes ? 4.f : 2.f);
    noButton.setOutlineThickness(hoverNo ? 4.f : 2.f);
    yesButton.setOutlineColor(sf::Color(0,0,0,180));
    noButton.setOutlineColor(sf::Color(0,0,0,180));
    window.draw(yesButton);
    window.draw(noButton);

    sf::Text yesText(font, "SI", 30);
    sf::FloatRect yesBounds = yesText.getLocalBounds();
    yesText.setOrigin(sf::Vector2f(yesBounds.position.x + yesBounds.size.x/2.f, yesBounds.position.y + yesBounds.size.y/2.f));
    yesText.setPosition(yesButton.getPosition() + sf::Vector2f(btnSize.x/2.f, btnSize.y/2.f));
    sf::Text noText(font, "NO", 30);
    sf::FloatRect noBounds = noText.getLocalBounds();
    noText.setOrigin(sf::Vector2f(noBounds.position.x + noBounds.size.x/2.f, noBounds.position.y + noBounds.size.y/2.f));
    noText.setPosition(noButton.getPosition() + sf::Vector2f(btnSize.x/2.f, btnSize.y/2.f));
    window.draw(yesText);
    window.draw(noText);
}

// Hit-test dei pulsanti SI/NO nel popup conferma ritorno
enum class ConfirmHit { None, Yes, No };

inline ConfirmHit confirmReturnHomeHitTest(
    const sf::Vector2u& windowSize,
    const sf::Vector2i& mousePos
) {
    sf::Vector2f center(windowSize.x/2.f, windowSize.y/2.f);
    sf::Vector2f popupSize(600.f,300.f);
    sf::FloatRect popup(sf::Vector2f(center.x - popupSize.x/2.f, center.y - popupSize.y/2.f), popupSize);
    sf::Vector2f btnSize(160.f,60.f);
    float gap = 80.f;
    float totalBtnWidth = btnSize.x*2 + gap;
    float startX = center.x - totalBtnWidth/2.f;
    float bottomMargin = 60.f;
    float buttonY = popup.size.x + popup. - btnSize.y - bottomMargin;
    sf::FloatRect yesRect(sf::Vector2f(startX, buttonY), btnSize);
    sf::FloatRect noRect(sf::Vector2f(startX + btnSize.x + gap, buttonY), btnSize);
    sf::Vector2f mposF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    if (yesRect.contains(mposF)) return ConfirmHit::Yes;
    if (noRect.contains(mposF)) return ConfirmHit::No;
    return ConfirmHit::None;
}

// Overlay prompt risposta (lista elementi con selezione evidenziata)
inline void drawResponsePromptOverlay(
    sf::RenderWindow& window,
    const sf::Font& font,
    const sf::Vector2u& windowSize,
    const std::vector<std::string>& items,
    size_t selectedIndex
) {
    sf::Vector2f panelSize(520.f, 180.f);
    sf::Vector2f panelPos( (windowSize.x - panelSize.x)/2.f, 100.f );
    sf::RectangleShape panel(panelSize);
    panel.setPosition(panelPos);
    panel.setFillColor(sf::Color(25,25,25,235));
    panel.setOutlineColor(sf::Color(255,200,90));
    panel.setOutlineThickness(3.f);
    window.draw(panel);
    sf::Text title(font, "Attivare una carta? (Invio conferma, ESC annulla)", 22);
    title.setFillColor(sf::Color(255,230,140));
    title.setOutlineColor(sf::Color(0,0,0,200));
    title.setOutlineThickness(2.f);
    title.setPosition(panelPos + sf::Vector2f(16.f, 10.f));
    window.draw(title);
    float y0 = panelPos.y + 50.f;
    for(size_t i=0;i<items.size();++i){
        sf::Text item(font, items[i], 22);
        item.setFillColor(i==selectedIndex ? sf::Color(255,240,160) : sf::Color(230,230,230));
        item.setOutlineColor(sf::Color(0,0,0,200));
        item.setOutlineThickness(2.f);
        item.setPosition(sf::Vector2f(panelPos.x + 20.f, y0 + static_cast<float>(i)*28.f));
        window.draw(item);
    }
}

// Overlay scelta DeckSend (griglia di carte, evidenzia selezionata, hint)
inline void drawDeckSendOverlay(
    sf::RenderWindow& window,
    const sf::Font& font,
    const sf::Vector2u& windowSize,
    const std::vector<Card>& candidates,
    size_t selectedIndex,
    const sf::Vector2f& slotSize,
    TextureManager& textureManager,
    float scale = 0.7f,
    float pad = 12.f
) {
    // Overlay semi-trasparente
    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
    overlay.setFillColor(sf::Color(0,0,0,160));
    window.draw(overlay);
    // Pannello e titolo
    sf::Vector2f panelSize(windowSize.x*0.86f, windowSize.y*0.70f);
    sf::Vector2f panelPos((windowSize.x - panelSize.x)/2.f, (windowSize.y - panelSize.y)/2.f);
    sf::RectangleShape panel(panelSize);
    panel.setPosition(panelPos);
    panel.setFillColor(sf::Color(25,25,25,235));
    panel.setOutlineColor(sf::Color(255,200,90));
    panel.setOutlineThickness(3.f);
    window.draw(panel);
    sf::Text title(font, "Scegli un Mostro Drago dal Deck da mandare al Cimitero", 30);
    title.setFillColor(sf::Color(255,230,140));
    title.setOutlineColor(sf::Color(0,0,0,200));
    title.setOutlineThickness(2.f);
    sf::FloatRect tB = title.getLocalBounds();
    title.setOrigin(sf::Vector2f(tB.position.x + tB.size.x/2.f, tB.position.y + tB.size.y/2.f));
    title.setPosition(sf::Vector2f(panelPos.x + panelSize.x/2.f, panelPos.y + 48.f));
    window.draw(title);
    // Griglia carte
    size_t cols = 5;
    sf::Vector2f gridPos(panelPos.x + 24.f, panelPos.y + 90.f);
    sf::Vector2f cellSize(slotSize.x*scale, slotSize.y*scale);
    for(size_t i=0;i<candidates.size();++i){
        size_t r = i/cols, c = i%cols;
        sf::Vector2f pos(gridPos.x + static_cast<float>(c)*(cellSize.x+pad), gridPos.y + static_cast<float>(r)*(cellSize.y+pad));
        Card copy = candidates[i];
        RenderUtils::applyFrontTextureIfAny(copy, textureManager);
        copy.setSize(cellSize);
        copy.setPosition(pos);
        sf::Color tint = (i==selectedIndex) ? sf::Color(255,240,160) : sf::Color::White;
        copy.draw(window, tint);
    }
    // Hint tasti
    sf::Text hint(font, "Invio = Conferma, ESC = Annulla, Frecce o Click per selezionare", 22);
    hint.setFillColor(sf::Color(230,230,230,240));
    hint.setOutlineColor(sf::Color(0,0,0,200));
    hint.setOutlineThickness(2.f);
    sf::FloatRect hB = hint.getLocalBounds();
    hint.setOrigin(sf::Vector2f(hB.position.x + hB.size.x/2.f, hB.position.y + hB.size.y/2.f));
    hint.setPosition(sf::Vector2f(panelPos.x + panelSize.x/2.f, panelPos.y + panelSize.y - 36.f));
    window.draw(hint);
}

// Hit-test dell'indice selezionato nell'overlay DeckSend (coerente con il layout usato in drawDeckSendOverlay)
inline std::optional<size_t> deckSendHitIndex(
    const sf::Vector2u& windowSize,
    const sf::Vector2f& slotSize,
    size_t candidateCount,
    const sf::Vector2i& mousePos,
    float scale = 0.7f,
    float pad = 12.f,
    size_t cols = 5
) {
    sf::Vector2f panelSize(windowSize.x*0.86f, windowSize.y*0.70f);
    sf::Vector2f panelPos((windowSize.x - panelSize.x)/2.f, (windowSize.y - panelSize.y)/2.f);
    sf::Vector2f gridPos(panelPos.x + 24.f, panelPos.y + 90.f);
    sf::Vector2f cellSize(slotSize.x*scale, slotSize.y*scale);
    sf::Vector2f mposF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    for(size_t i=0;i<candidateCount;++i){
        size_t r = i/cols, c = i%cols;
        sf::Vector2f pos(gridPos.x + static_cast<float>(c)*(cellSize.x+pad), gridPos.y + static_cast<float>(r)*(cellSize.y+pad));
        sf::FloatRect rect(pos, cellSize);
        if(rect.contains(mposF)) return i;
    }
    return std::nullopt;
}

} // namespace RenderUtils
