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

void DiscardController::update(float dt, Game& game) {
    if (animations.empty()) return;

    for (auto& a : animations) {
        a.time += dt / a.duration;
        if (a.time > 1.f) a.time = 1.f;
        float moveSpeed = 2000.f
        sf::Vector2f pos = a.start + (a.end - a.start) * moveSpeed;
        a.card.setPosition(pos);
    }

    for(auto& a : animations){

    }

    if (std::all_of(animations.begin(), animations.end(), [](const DiscardAnim& d) { return d.time >= 1.f; })) {
        auto& gy = game.getGraveyard();
        for (auto& c : pending) gy.push_back(c);
        pending.clear();
        animations.clear();
    }
}