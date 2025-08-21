#include "DiscardController.h"

DiscardController::DiscardController() : animations({}), pending({}) { }

void DiscardController::start(std::vector<Card>&& cards, sf::Vector2f graveyardCenter, float duration) {
    animations.clear();
    pending.clear();
    for (auto& c : cards) {
        DiscardAnim anim{ c, c.getPosition(), graveyardCenter, 0.f, duration };
        animations.push_back(anim);
        pending.push_back(c);
    }
}

bool DiscardController::active() const {
    return !animations.empty();
}

void DiscardController::