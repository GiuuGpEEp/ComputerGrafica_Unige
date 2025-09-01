#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <functional>
#include <vector>
#include <optional>
#include <string>
#include <algorithm>
#include "../Card/Card.h"
#include "../TextureManager/TextureManager.h"
#include "Config.h"
#include "auxFunc.h"
#include "../GameLogic/Game/Game.h"
#include "../Field/Field.h"

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
    // Preserve the current on-screen size so swapping the texture doesn't change the visual scale/position
    sf::FloatRect currentBounds = c.getGlobalBounds();
    sf::Texture &back = AppConfig::findTextureInMap(texMap, AppConfig::TextureKey::CardBack);
    c.setTexture(back);
    sf::Vector2u ts = back.getSize();
    c.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i((int)ts.x, (int)ts.y)));
    // If we had a valid displayed size, force the new texture to use the same on-screen size
    if (currentBounds.position.x > 0.f && currentBounds.position.y > 0.f) {
        c.setSize(sf::Vector2f(currentBounds.position.x, currentBounds.height));
    }
}

// Posiziona e centra la carta in uno slot, senza rotazione
inline void placeCentered(Card& c, const sf::Vector2f& slotPos, const sf::Vector2f& slotSize,
                          float scaleX = 1.f, float scaleY = 1.f) {
    // Ensure sprite uses top-left origin and no residual rotation from previous uses
    c.resetOrigin();
    c.setRotation(0.f);
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
// (funzione rimossa per evitare dipendenze su Game/Field/Type nell'header)

// Varie: mix di due colori (media dei canali, alpha del primo)
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
// Funzioni di utilità per il disegno in batch
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
// Zone sync helpers (spostati da main.cpp)
// =====================
// Sincronizza la rappresentazione grafica della zona mostri (P1 in basso, P2 in alto)
inline void syncMonsterZoneToField(
    Game* game,
    Field& field,
    std::vector<Card>& fieldCards,
    std::vector<Card>& oppFieldCards,
    TextureManager& textureManager,
    const sf::Vector2f& slotSize,
    bool &selectedCardIsOnField,
    std::optional<size_t> &selectedCardIndex
){
    if (!game) return;
    const int P1 = 1;
    const int P2 = 2;
    int cur = game->getTurn().getCurrentPlayerIndex();
    const auto &p1Zone = (cur == 0) ? game->getMonsterZone() : game->getOpponentMonsterZone();
    const auto &p2Zone = (cur == 0) ? game->getOpponentMonsterZone() : game->getMonsterZone();

    // Reset occupazione visiva degli slot Mostro per P1 e P2
    for (int s = 0; s < 3; ++s) {
        sf::Vector2f pos1 = field.getSlotPosition(Type::Monster, P1, s);
        if (auto sl = field.getSlotByPosition(pos1)) sl->setOccupied(false);
        sf::Vector2f pos2 = field.getSlotPosition(Type::Monster, P2, s);
        if (auto sl2 = field.getSlotByPosition(pos2)) sl2->setOccupied(false);
    }

    fieldCards.clear();
    float fieldCardScale = RenderUtils::FieldScale;
    sf::Vector2f fieldCardSize(slotSize.x * RenderUtils::FieldScale, slotSize.y * RenderUtils::FieldScale);
    for (size_t i = 0; i < p1Zone.size(); ++i) {
        Card c = p1Zone[i];
        bool isFD = game->isFaceDownAt(0, i);
        if (!isFD) { RenderUtils::applyFrontTextureIfAny(c, textureManager); }
        int slotIdx = game->getMonsterSlotIndexAt(0, i);
        if (slotIdx < 0) slotIdx = static_cast<int>(i);
        sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, P1, slotIdx);
        RenderUtils::placeCentered(c, slotPos, slotSize, fieldCardScale, fieldCardScale);
        fieldCards.push_back(c);
        if (auto sl = field.getSlotByPosition(slotPos)) sl->setOccupied(true);
    }
    oppFieldCards.clear();
    for (size_t i = 0; i < p2Zone.size(); ++i) {
        Card c = p2Zone[i];
        bool isFD2 = game->isFaceDownAt(1, i);
        if (!isFD2) { RenderUtils::applyFrontTextureIfAny(c, textureManager); }
        int slotIdx2 = game->getMonsterSlotIndexAt(1, i);
        if (slotIdx2 < 0) slotIdx2 = static_cast<int>(i);
        sf::Vector2f slotPos = field.getSlotPosition(Type::Monster, P2, slotIdx2);
        RenderUtils::placeCentered(c, slotPos, slotSize, fieldCardScale, fieldCardScale);
        oppFieldCards.push_back(c);
        if (auto sl = field.getSlotByPosition(slotPos)) sl->setOccupied(true);
    }
    if (selectedCardIsOnField && selectedCardIndex.has_value() && selectedCardIndex.value() >= fieldCards.size()) {
        selectedCardIndex.reset();
        selectedCardIsOnField = false;
    }
}

// Sincronizza M/T e Field Spell con lo stato logico
inline void syncSpellTrapZones(
    Game* game,
    Field& field,
    std::vector<Card>& p1STCards,
    std::vector<Card>& p2STCards,
    std::optional<Card>& p1FieldSpellCard,
    std::optional<Card>& p2FieldSpellCard,
    TextureManager& textureManager,
    const sf::Vector2f& slotSize
){
    if (!game) return;
    const int P1 = 1;
    const int P2 = 2;
    int cur = game->getTurn().getCurrentPlayerIndex();
    const auto &p1ST = (cur==0) ? game->getSpellTrapZone() : game->getOpponentSpellTrapZone();
    const auto &p2ST = (cur==0) ? game->getOpponentSpellTrapZone() : game->getSpellTrapZone();

    for (int s = 0; s < 3; ++s) {
        if (auto sl = field.getSlotByPosition(field.getSlotPosition(Type::SpellTrap, P1, s))) sl->setOccupied(false);
        if (auto sl = field.getSlotByPosition(field.getSlotPosition(Type::SpellTrap, P2, s))) sl->setOccupied(false);
    }
    if (auto sl = field.getSlotByPosition(field.getSlotPosition(Type::FieldSpell, P1))) sl->setOccupied(false);
    if (auto sl = field.getSlotByPosition(field.getSlotPosition(Type::FieldSpell, P2))) sl->setOccupied(false);

    p1STCards.clear(); p2STCards.clear();
    for (size_t i = 0; i < p1ST.size(); ++i) {
        Card c = p1ST[i];
        bool isFD = game->isSpellTrapFaceDownAt(0, i);
        if (!isFD) { RenderUtils::applyFrontTextureIfAny(c, textureManager); }
        sf::Vector2f slotPos = field.getSlotPosition(Type::SpellTrap, P1, (int)i);
        RenderUtils::placeCentered(c, slotPos, slotSize, RenderUtils::STScale, RenderUtils::STScale);
        p1STCards.push_back(c);
        if (auto sl = field.getSlotByPosition(slotPos)) sl->setOccupied(true);
    }
    for (size_t i = 0; i < p2ST.size(); ++i) {
        Card c = p2ST[i];
        bool isFD = game->isSpellTrapFaceDownAt(1, i);
        if (!isFD) { RenderUtils::applyFrontTextureIfAny(c, textureManager); }
        sf::Vector2f slotPos = field.getSlotPosition(Type::SpellTrap, P2, (int)i);
        RenderUtils::placeCentered(c, slotPos, slotSize, RenderUtils::STScale, RenderUtils::STScale);
        p2STCards.push_back(c);
        if (auto sl = field.getSlotByPosition(slotPos)) sl->setOccupied(true);
    }
    p1FieldSpellCard = game->getFieldSpellOf(0);
    if (p1FieldSpellCard.has_value()) {
        Card &c = p1FieldSpellCard.value();
        RenderUtils::applyFrontTextureIfAny(c, textureManager);
        sf::Vector2f pos = field.getSlotPosition(Type::FieldSpell, P1);
        RenderUtils::placeCentered(c, pos, slotSize, RenderUtils::FieldSpellScale, RenderUtils::FieldSpellScale);
        if (auto sl = field.getSlotByPosition(pos)) sl->setOccupied(true);
    }
    p2FieldSpellCard = game->getFieldSpellOf(1);
    if (p2FieldSpellCard.has_value()) {
        Card &c = p2FieldSpellCard.value();
        RenderUtils::applyFrontTextureIfAny(c, textureManager);
        sf::Vector2f pos = field.getSlotPosition(Type::FieldSpell, P2);
        RenderUtils::placeCentered(c, pos, slotSize, RenderUtils::FieldSpellScale, RenderUtils::FieldSpellScale);
        if (auto sl = field.getSlotByPosition(pos)) sl->setOccupied(true);
    }
}

// Wrapper che chiama entrambe
inline void syncZones(
    Game* game,
    Field& field,
    std::vector<Card>& fieldCards,
    std::vector<Card>& oppFieldCards,
    std::vector<Card>& p1STCards,
    std::vector<Card>& p2STCards,
    std::optional<Card>& p1FieldSpellCard,
    std::optional<Card>& p2FieldSpellCard,
    TextureManager& textureManager,
    const sf::Vector2f& slotSize,
    bool &selectedCardIsOnField,
    std::optional<size_t> &selectedCardIndex
){
    syncMonsterZoneToField(game, field, fieldCards, oppFieldCards, textureManager, slotSize, selectedCardIsOnField, selectedCardIndex);
    syncSpellTrapZones(game, field, p1STCards, p2STCards, p1FieldSpellCard, p2FieldSpellCard, textureManager, slotSize);
}

// =====================
// Funzioni di utilità per gli overlay
// =====================
// Disegna una singola carta Mostro nello slot specificato applicando rotazioni/texture di retro dove richiesto
// sideTop=false => lato basso (P1), sideTop=true => lato alto (P2)
inline void drawMonsterCard(
    sf::RenderWindow& window,
    Card cardCopy,
    bool isOpponentSide,
    const sf::Vector2f& slotPos,
    const sf::Vector2f& slotSize,
    bool isDefense,
    bool isFaceDown,
    bool isSelectedAttacker,
    bool showAttackDot,
    bool isHighlighted,
    const std::unordered_map<AppConfig::TextureKey, sf::Texture*>& texMap
) {
    sf::Color color = sf::Color::White;
    if (isSelectedAttacker) {
        color = sf::Color(255,240,160);
    }
    if(isHighlighted){
        color = RenderUtils::mixColors(color, sf::Color(160,200,255,color.a));
    }
    if (isFaceDown) {
        RenderUtils::applyBackTexture(cardCopy, texMap);
    }
    if (isDefense || isFaceDown) {
        RenderUtils::placeRotatedDefense(cardCopy, slotPos, slotSize);
    } else {
        if (!isOpponentSide) {
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

// Disegna l'overlay di Game Over con pannello centrale, titolo e un suggerimento
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

    // Suggerimento
    sf::Text hint(font, hintStr, 26);
    hint.setFillColor(sf::Color(240,240,240,240));
    hint.setOutlineColor(sf::Color(0,0,0,200));
    hint.setOutlineThickness(2.f);
    sf::FloatRect hB = hint.getLocalBounds();
    hint.setOrigin(sf::Vector2f(hB.position.x + hB.size.x/2.f, hB.position.y + hB.size.y/2.f));
    hint.setPosition(sf::Vector2f(center.x, pos.y + panelSize.y - 70.f));
    window.draw(hint);
}

// Overlay di conferma per il ritorno alla Home (con evidenziazione al passaggio del mouse sui pulsanti)
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

// Rilevamento click dei pulsanti SI/NO nel popup di conferma per il ritorno
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
    float buttonY = popup.position.y + popup.size.y - btnSize.y - bottomMargin;
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

// Overlay scelta DeckSend (griglia di carte, evidenzia selezionata, suggerimento)
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
    // Suggerimento tasti
    sf::Text hint(font, "Invio = Conferma, ESC = Annulla, Frecce o Click per selezionare", 22);
    hint.setFillColor(sf::Color(230,230,230,240));
    hint.setOutlineColor(sf::Color(0,0,0,200));
    hint.setOutlineThickness(2.f);
    sf::FloatRect hB = hint.getLocalBounds();
    hint.setOrigin(sf::Vector2f(hB.position.x + hB.size.x/2.f, hB.position.y + hB.size.y/2.f));
    hint.setPosition(sf::Vector2f(panelPos.x + panelSize.x/2.f, panelPos.y + panelSize.y - 36.f));
    window.draw(hint);
}

// Rilevamento click dell'indice selezionato nell'overlay DeckSend (coerente con il layout usato in drawDeckSendOverlay)
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

// =====================
// Funzioni di utilità per il disegno della mano
// =====================
// Disegna la mano del player in basso applicando l'offset verticale di sollevamento
inline void drawPlayerHand(
    sf::RenderWindow& window,
    const std::vector<Card>& hand,
    bool ssChoiceActive
) {
    for (size_t i = 0; i < hand.size(); ++i) {
        const Card& src = hand[i];
        Card c = src; // copia per non alterare lo stato originale
        float offset = ssChoiceActive ? 0.f : src.getOffset();
        c.setPosition(src.getPosition() - sf::Vector2f(0.f, offset));
        c.draw(window);
    }
}

// =====================
// Funzioni di utilità per l'HUD
// =====================
inline void drawTopLeftHud(
    sf::RenderWindow& window,
    const sf::Font& font,
    const std::string& text,
    const sf::Vector2f& pos = sf::Vector2f(AppConfig::Ui::PadLeft, AppConfig::Ui::TopY),
    unsigned int charSize = AppConfig::Ui::HudSize
) {
    sf::Text hud(font, text, charSize);
    hud.setFillColor(sf::Color(250,250,250,220));
    hud.setOutlineColor(sf::Color(0,0,0,180));
    hud.setOutlineThickness(2.f);
    hud.setPosition(pos);
    window.draw(hud);
}

inline void drawLpHud(
    sf::RenderWindow& window,
    const sf::Font& font,
    const sf::Vector2u& windowSize,
    int lpCur,
    int lpOpp,
    unsigned int charSize = AppConfig::Ui::HudSize
) {
    std::string lpHud = std::string("LP ") + std::to_string(lpCur) + " | Opp " + std::to_string(lpOpp);
    sf::Text lpText(font, lpHud, charSize);
    lpText.setFillColor(sf::Color(255,230,140));
    lpText.setOutlineColor(sf::Color(0,0,0,180));
    lpText.setOutlineThickness(2.f);
    sf::FloatRect lpBounds = lpText.getLocalBounds();
    lpText.setPosition(sf::Vector2f(windowSize.x - lpBounds.size.x - 25.f, 10.f));
    window.draw(lpText);
}

inline void drawTributePrompt(sf::RenderWindow& window, const sf::Font& font) {
    sf::Text t(font, "Seleziona tributi (ESC annulla)", AppConfig::Ui::SmallSize);
    t.setFillColor(sf::Color(255,200,90,240));
    t.setOutlineColor(sf::Color(0,0,0,180));
    t.setOutlineThickness(2.f);
    t.setPosition(sf::Vector2f(AppConfig::Ui::PadLeft, AppConfig::Ui::TribY));
    window.draw(t);
}

inline void drawHandOverflowWarn(sf::RenderWindow& window, const sf::Font& font) {
    sf::Text warn(font, "Limite mano superato: scarterai in End Phase", AppConfig::Ui::SmallSize);
    warn.setFillColor(sf::Color(255,80,80,240));
    warn.setOutlineColor(sf::Color(0,0,0,200));
    warn.setOutlineThickness(2.f);
    warn.setPosition(sf::Vector2f(AppConfig::Ui::PadLeft, AppConfig::Ui::WarnY));
    window.draw(warn);
}

// Disegna il messaggio di avviso quando non è stato selezionato un deck (con fade-out)
inline void drawNoDeckWarning(
    sf::RenderWindow& window,
    sf::Text& textObj,
    const sf::Vector2u& windowSize,
    float elapsed,
    float duration,
    float fadeTime
){
    if(elapsed >= duration) return;
    sf::FloatRect bounds = textObj.getLocalBounds();
    textObj.setPosition(sf::Vector2f((windowSize.x - bounds.size.x)/2.f, windowSize.y * 0.78f));
    // Fade negli ultimi `fadeTime` secondi
    float alpha = 255.f;
    if(duration - elapsed < fadeTime){
        alpha = 255.f * (duration - elapsed) / fadeTime;
    }
    sf::Color color = textObj.getFillColor();
    color.a = static_cast<uint8_t>(std::max(0.f, std::min(255.f, alpha)));
    textObj.setFillColor(color);
    window.draw(textObj);
}

inline void drawSSChoicePrompt(sf::RenderWindow& window, const sf::Font& font) {
    sf::Text prompt(font, "Evocazione Speciale: A = Attacco, D = Difesa (mai coperto)", AppConfig::Ui::HudSize);
    prompt.setFillColor(sf::Color(255,230,140,245));
    prompt.setOutlineColor(sf::Color(0,0,0,200));
    prompt.setOutlineThickness(2.f);
    prompt.setPosition(sf::Vector2f(AppConfig::Ui::PadLeft, AppConfig::Ui::SSChoiceY));
    window.draw(prompt);
}

inline void drawSTChoicePrompt(sf::RenderWindow& window, const sf::Font& font) {
    sf::Text prompt(font, "Magia/Trappola: A = Attiva, S = Setta", AppConfig::Ui::SmallSize);
    prompt.setFillColor(sf::Color(255,230,140,245));
    prompt.setOutlineColor(sf::Color(0,0,0,200));
    prompt.setOutlineThickness(2.f);
    prompt.setPosition(sf::Vector2f(AppConfig::Ui::PadLeft, AppConfig::Ui::STChoiceY));
    window.draw(prompt);
}

inline void drawBattleHint(sf::RenderWindow& window, const sf::Font& font) {
    sf::Text hint(font, "Battle: clicca un tuo mostro (rossi = gia' usati), tasto A = attacco diretto", AppConfig::Ui::SmallSize);
    hint.setFillColor(sf::Color(200,230,255,235));
    hint.setOutlineColor(sf::Color(0,0,0,180));
    hint.setOutlineThickness(2.f);
    hint.setPosition(sf::Vector2f(AppConfig::Ui::PadLeft, AppConfig::Ui::WarnY));
    window.draw(hint);
}

inline void drawCenterFeedback(
    sf::RenderWindow& window,
    const sf::Font& font,
    const sf::Vector2u& windowSize,
    const std::string& text,
    unsigned int charSize = AppConfig::Ui::HudSize,
    float y = AppConfig::Ui::CenterFeedbackY
) {
    sf::Text fb(font, text, charSize);
    fb.setFillColor(sf::Color(255,200,90,235));
    fb.setOutlineColor(sf::Color(0,0,0,180));
    fb.setOutlineThickness(2.f);
    sf::FloatRect b = fb.getLocalBounds();
    fb.setPosition(sf::Vector2f((windowSize.x - b.size.x)/2.f, y));
    window.draw(fb);
}

// Disegna il pannello dei dettagli della carta (usato per la carta pescata e per la carta selezionata)
inline void drawDetailsPanel(
    sf::RenderWindow& window,
    const Card& card,
    const sf::Font& font,
    const sf::Vector2u& windowSize,
    float scrollOffset
){
    sf::Vector2f panelPos{AppConfig::Ui::DetailsPanelPosX, AppConfig::Ui::DetailsPanelPosY};
    sf::Vector2f panelSize{AppConfig::Ui::DetailsPanelWidth, AppConfig::Ui::DetailsPanelHeight};
    showCardDetails(window, card, font, panelPos, panelSize, scrollOffset);
}

// Disegna il pannello dei dettagli usato dalle sovrapposizioni (Extra/Cimitero)
inline void drawOverlayDetails(
    sf::RenderWindow& window,
    const Card& card,
    const sf::Font& font,
    const sf::Vector2u& windowSize,
    float scrollOffset
){
    sf::Vector2f panelPos{ windowSize.x * AppConfig::Ui::OverlayDetailsPosXFactor, windowSize.y * AppConfig::Ui::OverlayDetailsPosYFactor };
    sf::Vector2f panelSize{ AppConfig::Ui::OverlayDetailsWidth, AppConfig::Ui::OverlayDetailsHeight };
    showCardDetails(window, card, font, panelPos, panelSize, scrollOffset);
}

// Disegna la mano dell'avversario (in alto): sempre coperta e ruotata 180°
inline void drawOpponentHand(
    sf::RenderWindow& window,
    const std::vector<Card>& hand,
    const sf::Vector2u& windowSize,
    const sf::Vector2f& cardSize,
    float spacing,
    const std::unordered_map<AppConfig::TextureKey, sf::Texture*>& texMap,
    float topMargin = AppConfig::Ui::OpponentHandTopMarginPx
) {
    size_t n = hand.size();
    if(n == 0) return;
    float totalW = n * cardSize.x + (n - 1) * spacing;
    float startX = (windowSize.x - totalW) / 2.f;
    for(size_t i=0;i<n;++i){
        Card c = hand[i];
        RenderUtils::applyBackTexture(c, texMap);
        sf::Vector2f pos(startX + static_cast<float>(i) * (cardSize.x + spacing), topMargin);
        RenderUtils::rotate180Centered(c, pos, cardSize);
        c.draw(window);
    }
}

} // namespace RenderUtils
