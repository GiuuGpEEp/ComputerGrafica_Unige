#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include "../Card/Card.h"
#include "../TextureManager/TextureManager.h"
#include "../../resources/data/GameState.h"

struct Config {
        float holdThreshold = 0.45f;
        int   maxCols = 8;
        float previewScale = 0.95f;
        float hSpacing = 18.f;
        float vSpacing = 28.f;
        float bottomPadding = 90.f;
    };

class ExtraDeckOverlay {
public:
    ExtraDeckOverlay(const sf::Vector2u& windowSize, const sf::Vector2f& slotSize, TextureManager& texMgr, const Config& cfg = Config());

    void startHold(const sf::Vector2f& mousePos, const sf::FloatRect& extraSlotBounds, GameState state);
    void update(float deltaTime, bool isDragging, GameState state);

    void handleMousePress(const sf::Vector2f& mousePos, const std::vector<Card>& extraCards);
    void handleMouseRelease();
    void handleScroll(float delta);
    void handleEscape();

    void draw(sf::RenderWindow& window, const sf::Font& font, const std::vector<Card>& extraCards, float scrollOffset);

    bool isOverlayVisible() const;
    bool detailsActive() const;
    float getScrollOffset() const;
    void setScrollOffset(float v);

    const Card* getSelectedCard() const { return detailsShown ? &selectedCard : nullptr; }

private:
    // state
    bool holding = false;
    bool showOverlay = false;
    bool detailsShown = false;

    sf::Clock holdClock;
    sf::Vector2u windowSize;
    sf::Vector2f slotSize;

    TextureManager& textureManager;
    Config config;

    // layout + interaction
    sf::FloatRect overlayBounds; // backdrop
    std::vector<sf::FloatRect> cardBounds;
    Card selectedCard; // copia
    float scrollOffset = 0.f; // riuso stesso offset esterno

    // internal helpers
    void openOverlay();
    void closeOverlay();
};
