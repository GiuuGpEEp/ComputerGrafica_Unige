#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "../Card/Card.h"

struct ShuffleCard {
    Card shuffledCard;
    sf::Vector2f startPos;
    sf::Vector2f targetPos;
    float progress = 0.f;
    bool returning = false;
};

class ShuffleAnimation {
public:
    
    ShuffleAnimation();
    void start(const std::vector<Card>& deck, sf::Vector2f deckPos, size_t numCards = 10);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    bool isFinished() const;
    void clear();

private:
    std::vector<ShuffleCard> shuffleCards;
    bool finished = false;
};
