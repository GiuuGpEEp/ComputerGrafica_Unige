#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include "../Card/Card.h"
#include "../TextureManager/TextureManager.h"
#include "../../resources/data/GameState.h"

class GraveyardOverlay {
public:
    struct Config {
        float holdThreshold = 0.45f;
        int   maxCols = 8;
        float previewScale = 0.95f;
        float hSpacing = 18.f;
        float vSpacing = 28.f;
        float bottomPadding = 90.f;
        bool  newestOnTop = true; // se true mostra ultima carta per prima (indice 0)
    };

    GraveyardOverlay(const sf::Vector2u& windowSize, const sf::Vector2f& slotSize, TextureManager& texMgr, const Config& cfg = Config{});

    void startHold(const sf::Vector2f& mousePos, const sf::FloatRect& gySlotBounds, GameState state, bool hasCards);
    void update(float deltaTime, bool isDragging, GameState state);

    void handleMousePress(const sf::Vector2f& mousePos, const std::vector<Card>& gyCards);
    void handleMouseRelease();
    void handleScroll(float delta);
    void handleEscape();

    void draw(sf::RenderWindow& window, const sf::Font& font, const std::vector<Card>& gyCards, float scrollOffsetExternal);

    bool isOverlayVisible() const; 
    bool detailsActive() const; 
    const Card* getSelectedCard() const; 

private:
    bool holding = false;
    bool showOverlay = false;
    bool detailsShown = false;

    sf::Clock holdClock;
    sf::Vector2u windowSize;
    sf::Vector2f slotSize;
    TextureManager& textureManager;
    Config config;

    sf::FloatRect overlayBounds;
    std::vector<sf::FloatRect> cardBounds;
    Card selectedCard;
    float scrollOffset = 0.f;

    void openOverlay();
    void closeOverlay();
};
