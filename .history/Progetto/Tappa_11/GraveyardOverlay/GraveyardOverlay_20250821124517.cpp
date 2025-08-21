#include "GraveyardOverlay.h"
#include <algorithm>

GraveyardOverlay::GraveyardOverlay(const sf::Vector2u& winSize, const sf::Vector2f& slotSz, TextureManager& tm, const Config& cfg)
: windowSize(winSize), slotSize(slotSz), textureManager(tm), config(cfg) {}

void GraveyardOverlay::startHold(const sf::Vector2f& mousePos, const sf::FloatRect& gySlotBounds, GameState state, bool hasCards){
    if(!hasCards) return; // niente da mostrare
    if(state == GameState::FieldVisible || state == GameState::Playing) {
        if(gySlotBounds.contains(mousePos)) {
            if(!showOverlay) { holding = true; holdClock.restart(); }
            else { holding = false; }
        }
    }
}

void GraveyardOverlay::update(float, bool isDragging, GameState state){
    if(holding && !isDragging && (state == GameState::FieldVisible || state == GameState::Playing)) {
        if(holdClock.getElapsedTime().asSeconds() >= config.holdThreshold) openOverlay();
    }
}

void GraveyardOverlay::handleMousePress(const sf::Vector2f& mousePos, const std::vector<Card>& gyCards){
    if(!showOverlay) return;
    if(!overlayBounds.contains(mousePos)) { closeOverlay(); return; }
    for(size_t i=0; i<cardBounds.size(); ++i){
        if(cardBounds[i].contains(mousePos)){
            size_t idx = i;
            if(idx < gyCards.size()){
                selectedCard = gyCards[idx];
                if(!selectedCard.originalTexturePath.empty()){
                    sf::Texture& frontTex = textureManager.getTexture(selectedCard.originalTexturePath);
                    selectedCard.setTexture(frontTex);
                    sf::Vector2u texSize = frontTex.getSize();
                    selectedCard.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i(texSize.x, texSize.y)));                }
                detailsShown = true; scrollOffset = 0.f;
            }
            break;
        }
    }
}

void GraveyardOverlay::handleMouseRelease(){ holding = false; }
void GraveyardOverlay::handleScroll(float delta){ if(detailsShown){ scrollOffset -= delta * 20.f; if(scrollOffset < 0.f) scrollOffset = 0.f; } }
void GraveyardOverlay::handleEscape(){ if(detailsShown) detailsShown = false; else if(showOverlay) closeOverlay(); }
void GraveyardOverlay::openOverlay(){ showOverlay = true; }
void GraveyardOverlay::closeOverlay(){ showOverlay = false; detailsShown = false; holding = false; }

void GraveyardOverlay::draw(sf::RenderWindow& window, const sf::Font& font, const std::vector<Card>& gyCards, float){
    if(!showOverlay || gyCards.empty()) return;

    // Prepara ordine (se newestOnTop mostrare in overlay carta più recente prima)
    std::vector<Card> ordered = gyCards;
    if(config.newestOnTop){ std::reverse(ordered.begin(), ordered.end()); }

    int total = static_cast<int>(ordered.size());
    int cols = std::min(config.maxCols, total);
    int rows = (total + cols -1)/cols;
    sf::Vector2f previewCardSize(slotSize.x * config.previewScale, slotSize.y * config.previewScale);
    float gridWidth = cols * previewCardSize.x + (cols -1)*config.hSpacing;
    float gridHeight = rows * previewCardSize.y + (rows -1)*config.vSpacing;
    sf::Vector2f origin((windowSize.x - gridWidth)/2.f, windowSize.y * 0.60f - gridHeight);

    sf::RectangleShape backdrop;
    backdrop.setPosition(origin - sf::Vector2f(20.f, 30.f));
    backdrop.setSize(sf::Vector2f(gridWidth + 40.f, gridHeight + 60.f + config.bottomPadding));
    backdrop.setFillColor(sf::Color(0,0,0,170));
    backdrop.setOutlineColor(sf::Color(255,255,255,120));
    backdrop.setOutlineThickness(2.f);
    window.draw(backdrop);
    overlayBounds = backdrop.getGlobalBounds();

    sf::Text title(font, "GRAVEYARD", 24);
    title.setFillColor(sf::Color::White);
    sf::FloatRect tB = title.getLocalBounds();
    title.setPosition(sf::Vector2f(origin.x + (gridWidth - tB.size.x)/2.f, backdrop.getPosition().y + 6.f));
    window.draw(title);

    cardBounds.clear(); cardBounds.reserve(total);
    for(int i=0; i<total; ++i){
        int r = i / cols; int c = i % cols;
        sf::Vector2f pos = origin + sf::Vector2f(c*(previewCardSize.x + config.hSpacing), r*(previewCardSize.y + config.vSpacing) + 30.f);
        Card previewCard = ordered[i];
        if(!previewCard.originalTexturePath.empty()){
            sf::Texture& frontTex = textureManager.getTexture(previewCard.originalTexturePath);
            previewCard.setTexture(frontTex);
            sf::Vector2u texSize = frontTex.getSize();
            previewCard.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i(texSize.x, texSize.y))); }
        previewCard.setPosition(pos);
        previewCard.setSize(previewCardSize);
        previewCard.draw(window);
        cardBounds.emplace_back(pos, previewCardSize);
    }

    std::string hintStr = detailsShown ? "ESC per chiudere i dettagli, click fuori per chiudere" : "Click carta per dettagli, fuori per chiudere";
    sf::Text hint(font, hintStr, 16);
    hint.setFillColor(sf::Color(220,220,220));
    sf::FloatRect hB = hint.getLocalBounds();
    hint.setPosition(sf::Vector2f(origin.x + (gridWidth - hB.size.x)/2.f, origin.y + gridHeight + 40.f));
    window.draw(hint);
}

bool GraveyardOverlay::isOverlayVisible() const { return showOverlay; }
bool GraveyardOverlay::detailsActive() const { return detailsShown; }
const Card* GraveyardOverlay::getSelectedCard() const { return detailsShown ? &selectedCard : nullptr; }
