#include "ExtraDeckOverlay.h"
#include <algorithm>

ExtraDeckOverlay::ExtraDeckOverlay(const sf::Vector2u& winSize, const sf::Vector2f& slotSz, TextureManager& tm, const Config& cfg)
: windowSize(winSize), slotSize(slotSz), textureManager(tm), config(cfg) {}

void ExtraDeckOverlay::startHold(const sf::Vector2f& mousePos, const sf::FloatRect& extraSlotBounds, GameState state) {
    if(state == GameState::FieldVisible || state == GameState::Playing) {
        if(extraSlotBounds.contains(mousePos)) {
            // Avvia il conteggio dell'hold solo se l'overlay non è già aperto.
            // Se è già aperto, non lo richiudiamo: resta visibile finché non si clicca fuori o ESC.
            if(!showOverlay) {
                holding = true;
                holdClock.restart();
            } else {
                // Overlay già aperto: interpretare il nuovo click come semplice interazione,
                // non come richiesta di chiuderlo/riaprirlo.
                holding = false; // nessun hold in corso
            }
        }
    }
}

void ExtraDeckOverlay::update(float deltaTime, bool isDragging, GameState state){
    (void)deltaTime; 
    if(holding && !isDragging && (state == GameState::FieldVisible || state == GameState::Playing)) {
        if(holdClock.getElapsedTime().asSeconds() >= config.holdThreshold) {
            openOverlay();
        }
    }
}

void ExtraDeckOverlay::handleMousePress(const sf::Vector2f& mousePos, const std::vector<Card>& extraCards){
    if(!showOverlay) return;
    if(!overlayBounds.contains(mousePos)) { // click fuori chiude
        closeOverlay();
        return;
    }
    // inside overlay: check card selection
    for(size_t i=0; i<cardBounds.size(); ++i){
        if(cardBounds[i].contains(mousePos)){
            if(i < extraCards.size()){
                selectedCard = extraCards[i];
                if(!selectedCard.originalTexturePath.empty()) {
                    sf::Texture& frontTex = textureManager.getTexture(selectedCard.originalTexturePath);
                    selectedCard.setTexture(frontTex);
                    sf::Vector2u ts = frontTex.getSize();
                    selectedCard.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i(ts.x, ts.y)));
                }
                detailsShown = true;
                scrollOffset = 0.f;
            }
            break;
        }
    }
}

void ExtraDeckOverlay::handleMouseRelease(){
    holding = false; // non chiudere automaticamente
}

void ExtraDeckOverlay::handleScroll(float delta){
    if(detailsShown){
        scrollOffset -= delta * 20.f;
        if(scrollOffset < 0.f) scrollOffset = 0.f;
    }
}

void ExtraDeckOverlay::handleEscape(){
    if(detailsShown) {
        detailsShown = false;
    } else if(showOverlay) {
        closeOverlay();
    }
}

void ExtraDeckOverlay::openOverlay(){ showOverlay = true; }
void ExtraDeckOverlay::closeOverlay(){ showOverlay = false; detailsShown = false; holding = false; }

void ExtraDeckOverlay::draw(sf::RenderWindow& window, const sf::Font& font, const std::vector<Card>& extraCards, float scrollOffsetExternal){
    if(!showOverlay || extraCards.empty()) return;

    // Layout calculation
    int total = static_cast<int>(extraCards.size());
    int cols = std::min(config.maxCols, total);
    int rows = (total + cols - 1)/cols;

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

    sf::Text title(font, "EXTRA DECK", 24);
    title.setFillColor(sf::Color::White);
    sf::FloatRect tB = title.getLocalBounds();
    title.setPosition(sf::Vector2f(origin.x + (gridWidth - tB.size.x)/2.f, backdrop.getPosition().y + 6.f));
    window.draw(title);

    cardBounds.clear();
    cardBounds.reserve(total);
    for(int i=0; i<total; ++i){
        int r = i / cols;
        int c = i % cols;
        sf::Vector2f pos = origin + sf::Vector2f(c*(previewCardSize.x + config.hSpacing), r*(previewCardSize.y + config.vSpacing) + 30.f);
        Card previewCard = extraCards[i];
        if(!previewCard.originalTexturePath.empty()){
            sf::Texture& frontTex = textureManager.getTexture(previewCard.originalTexturePath);
            previewCard.setTexture(frontTex);
            sf::Vector2u texSize = frontTex.getSize();
            previewCard.setTextureRect(sf::IntRect(sf::Vector2i(0,0), sf::Vector2i(texSize.x, texSize.y)));
        }
        previewCard.setPosition(pos);
        previewCard.setSize(previewCardSize);
        previewCard.draw(window);
        cardBounds.emplace_back(pos, previewCardSize);
    }

    std::string hintStr;
    if(detailsShown) hintStr = "ESC per chiudere i dettagli, click fuori per chiudere";
    else hintStr = "Click carta per dettagli, fuori per chiudere";

    sf::Text hint(font, hintStr, 16);
    hint.setFillColor(sf::Color(220,220,220));
    sf::FloatRect hB = hint.getLocalBounds();
    hint.setPosition(sf::Vector2f(origin.x + (gridWidth - hB.size.x)/2.f, origin.y + gridHeight + 40.f));
    window.draw(hint);

    // Dettagli carta
    if(detailsShown){
        sf::Vector2f panelPos{ windowSize.x * 0.07f, windowSize.y * 0.12f };
        sf::Vector2f panelSize{ 340.f, 260.f };
        // Usa funzione globale showCardDetails (dichiarata altrove) -> verrà chiamata dal main sfruttando selectedCard
        // Qui non la chiamiamo direttamente perché evitare dipendenze incrociate; main si occupa.
    }
}
