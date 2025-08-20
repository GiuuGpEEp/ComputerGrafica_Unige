#include "DrawController.h"

DrawController::DrawController() : cardsToDraw(0), showDrawnCardDetails(false) {}

void DrawController::queueDraw(int n) {
    cardsToDraw += n;
}

int DrawController::pending() const {
    return cardsToDraw;
}

void DrawController::update(
    float deltaTime,
    bool mousePressed,
    Deck& deck,
    std::vector<Card>& hand,
    sf::Vector2u windowSize,
    sf::Vector2f cardSize,
    float spacing,
    float handY,
    int handMaxSize,
    sf::Vector2f deckSlotPos,
    sf::Texture& textureFlipped,
    TextureManager& textureManager
){
    if (cardsToDraw > 0 && animations.empty() && !deck.isEmpty()) {
        Card next = deck.drawCard();
        --cardsToDraw;
        DrawAnimation anim(next, DrawAnimationPhases::MovingOut, deckSlotPos,
                           sf::Vector2f(windowSize.x / 2.f - cardSize.x / 2.f, windowSize.y / 2.f - cardSize.y / 2.f), textureManager);
        animations.push_back(anim);
    }
    if (!animations.empty()) {
        DrawAnimationPhases ph = animations.front().update(MOVEMENT_SPEED_INTERNAL, deltaTime, hand, windowSize, cardSize, spacing, handY, handMaxSize, mousePressed, textureManager);
        if (ph == DrawAnimationPhases::ShowCard) {
            showDrawnCardDetails = true;
            detailCardOpt = animations.front().getCard();
        }
        if (ph == DrawAnimationPhases::Done) {
            showDrawnCardDetails = false;
            detailCardOpt.reset();

            // Aggiunge al vettore mano (che rappresenta Player.hand)
            hand.push_back(animations.front().getCard());
            animations.erase(animations.begin());
            updateHandPositions(hand, windowSize, cardSize, spacing, handY, handMaxSize);
        }
    }
}

bool DrawController::isShowingDetails() const {
    return showDrawnCardDetails && detailCardOpt.has_value();
}

const Card& DrawController::detailCard() const {
    return detailCardOpt.value();
}

bool DrawController::idle() const {
    return animations.empty();
}

void DrawController::reset() {
    animations.clear();
    cardsToDraw = 0;
    showDrawnCardDetails = false;
    detailCardOpt.reset();
}

void DrawController::draw(sf::RenderWindow& window) {
    for (auto& anim : animations) {
        anim.draw(window);
    }
}