#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "../Game/Game.h"
#include "../../Card/Card.h"

class DiscardController {
public:
    DiscardController();
    void start(std::vector<Card>&& cards, sf::Vector2f graveyardCenter, float duration = 0.45f){
        animations.clear();
        pending.clear();
        for(auto &c : cards){
            DiscardAnim anim{c, c.getPosition(), graveyardCenter, 0.f, duration};
            animations.push_back(anim);
            pending.push_back(c);
        }
    }
    bool active() const { return !animations.empty(); }
    void update(float dt, Game& game);
    void draw(sf::RenderWindow& window);
private:
    struct DiscardAnim { Card card; sf::Vector2f start; sf::Vector2f end; float time; float duration; bool ended; };
    std::vector<DiscardAnim> animations; // animazioni in corso
    std::vector<Card> pending; // carte da aggiungere a graveyard quando animazione termina
};
