#include "ShuffleAnimation.h"
#include <cmath>

ShuffleAnimation::ShuffleAnimation() {}

void ShuffleAnimation::start(const std::vector<Card>& deck, sf::Vector2f deckPos, size_t numCards) {
    shuffleCards.clear();
    finished = false;
    size_t n = std::min(deck.size(), numCards);
    for (size_t i = 0; i < n; ++i) {
        ShuffleCard sc;
        sc.shuffledCard = deck[i];
        sc.shuffledCard.setPosition(deckPos);
        float offsetX = (i % 2 == 0 ? -1.f : 1.f) * (50.f + static_cast<float>(rand() % 30));
        float offsetY = -static_cast<float>(i) * 2.f;
        sc.startPos = deckPos;
        sc.targetPos = deckPos + sf::Vector2f(offsetX, offsetY);
        sc.progress = 0.f;
        sc.returning = false;
        shuffleCards.push_back(sc);
    }
}

void ShuffleAnimation::update(float dt) {
    finished = true;
    for (auto& sc : shuffleCards) {
        sf::Vector2f newPos;
        if (!sc.returning) {
            sc.progress += dt * 2.f;
            float t = std::sin(sc.progress * 3.14159f);
            newPos.x = sc.startPos.x + (sc.targetPos.x - sc.startPos.x) * t;
            newPos.y = sc.startPos.y + (sc.targetPos.y - sc.startPos.y) * t;
            sc.shuffledCard.setPosition(newPos);
            if (sc.progress >= 1.f) {
                sc.progress = 0.f;
                sc.returning = true;
            } else {
                finished = false;
            }
        } else {
            sc.progress += dt * 2.f;
            float t = std::sin(sc.progress * 3.14159f);
            newPos.x = sc.targetPos.x + (sc.startPos.x - sc.targetPos.x) * t;
            newPos.y = sc.targetPos.y + (sc.startPos.y - sc.targetPos.y) * t;
            sc.shuffledCard.setPosition(newPos);
            if (sc.progress >= 1.f) {
                sc.shuffledCard.setPosition(sc.startPos);
            } else {
                finished = false;
            }
        }
    }
}

void ShuffleAnimation::draw(sf::RenderWindow& window) {
    for (auto& sc : shuffleCards) {
        sc.shuffledCard.draw(window);
    }
}

bool ShuffleAnimation::isFinished() const {
    return finished;
}

void ShuffleAnimation::clear() {
    shuffleCards.clear();
    finished = true;
}
