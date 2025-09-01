#include "DiscardController.h"

DiscardController::DiscardController() : animations({}), pending({}) { }

void DiscardController::start(std::vector<Card>&& cards, sf::Vector2f graveyardCenter, float duration) {
    animations.clear();
    pending.clear();
    for (auto& c : cards) {
        DiscardAnim anim{ c, c.getPosition(), graveyardCenter, 0.f, duration, false};
        animations.push_back(anim);
        pending.push_back(c);
    }
}

bool DiscardController::active() const {
    return !animations.empty();
}

void DiscardController::update(float dt, Game& game) {
    if (animations.empty()) return;

    for (auto& a : animations) {
        a.time += dt / a.duration;
        if (a.time > 1.f) a.time = 1.f;
        float easing = 1.f - (1.f - a.time) * (1.f - a.time); // easeOutQuad
        sf::Vector2f pos = a.start + (a.end - a.start) * easing;
        a.card.setPosition(pos);
        if(a.time >= 1.f) a.ended = true; // marca conclusa solo al termine
    }

    int endCount = 0;
    for (const auto& a : animations) {
        if (a.ended) endCount++;
    }

    if (endCount == animations.size()) {
        auto& gy = game.getGraveyard();
        for (auto& c : pending) gy.push_back(c);
        pending.clear();
        animations.clear();
    }
}

void DiscardController::draw(sf::RenderWindow& window) {
    for (auto& a : animations) {
        a.card.draw(window);
    }
}