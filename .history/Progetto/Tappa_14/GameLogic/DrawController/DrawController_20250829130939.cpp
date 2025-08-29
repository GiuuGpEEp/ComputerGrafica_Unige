#include "DrawController.h"
#include "../../Utils/RenderUtils.h"

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
    bool forceSkipReveal,
    Deck& deck,
    std::vector<Card>& hand,
    sf::Vector2u windowSize,
    sf::Vector2f cardSize,
    float spacing,
    float handY,
    int handMaxSize,
    sf::Vector2f deckSlotPos,
    sf::Texture& textureFlipped,
    TextureManager& textureManager,
    EventDispatcher* dispatcher
){
    if (cardsToDraw > 0 && animations.empty() && !deck.isEmpty()) {
        Card next = deck.drawCard();
        --cardsToDraw;
    DrawAnimation anim(next, DrawAnimationPhases::MovingOut, deckSlotPos,
               sf::Vector2f(windowSize.x / 2.f - cardSize.x / 2.f, windowSize.y / 2.f - cardSize.y / 2.f), textureManager, forceSkipReveal);
    animations.push_back(anim);
    std::cout << "[DrawController] queued draw; forceSkipReveal=" << (forceSkipReveal?"1":"0") << " remaining=" << cardsToDraw << std::endl;
    if(dispatcher) dispatcher->emit(GameEventType::DrawStart);
    }
    if (!animations.empty()) {
    // mousePressed: allow early exit from center wait; forceSkipReveal: skip center reveal entirely
    DrawAnimationPhases ph = animations.front().update(MOVEMENT_SPEED_INTERNAL, deltaTime, textureFlipped, hand, windowSize, cardSize, spacing, handY, handMaxSize, mousePressed, forceSkipReveal, textureManager);
        if (ph == DrawAnimationPhases::ShowCard) {
            showDrawnCardDetails = true;
            detailCardOpt = animations.front().getCard();
        }
        if (ph == DrawAnimationPhases::Done) {
            showDrawnCardDetails = false;
            detailCardOpt.reset();

            // Prepare final card to push into hand: ensure front texture for visible hands,
            // or back texture for concealed hands (forceSkipReveal indicates opponent draw)
            Card finalCard = animations.front().getCard();
            if (forceSkipReveal) {
                finalCard.setTexture(textureFlipped);
                std::cout << "[DrawController] finalCard assigned BACK texture for card origPath='" << finalCard.originalTexturePath << "'" << std::endl;
            } else {
                RenderUtils::applyFrontTextureIfAny(finalCard, textureManager);
                std::cout << "[DrawController] finalCard applied FRONT texture for card origPath='" << finalCard.originalTexturePath << "'" << std::endl;
            }

            // Aggiunge al vettore mano (che rappresenta Player.hand)
            hand.push_back(std::move(finalCard));
            animations.erase(animations.begin());
            updateHandPositions(hand, windowSize, cardSize, spacing, handY, handMaxSize);
            if(dispatcher) dispatcher->emit(GameEventType::DrawEnd);
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