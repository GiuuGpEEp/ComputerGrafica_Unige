#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "../Game/Game.h"
#include "../../Card/Card.h"

class DiscardController {
public:
    

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
    void update(float dt, Game& game){
        if(animations.empty()) return;
        for(auto &a : animations){
            a.t += dt / a.duration; if(a.t > 1.f) a.t = 1.f;
            float e = 1.f - (1.f - a.t)*(1.f - a.t); // easeOutQuad
            sf::Vector2f pos = a.start + (a.end - a.start)*e;
            a.card.setPosition(pos);
        }
        // Se tutte concluse, scarica nel graveyard
        if(std::all_of(animations.begin(), animations.end(), [](const DiscardAnim &d){ return d.t >= 1.f; })){
            auto &gy = game.getGraveyard();
            for(auto &c : pending) gy.push_back(c);
            pending.clear();
            animations.clear();
        }
    }
    void draw(sf::RenderWindow& window){ for(auto &a : animations) a.card.draw(window); }
private:
    struct DiscardAnim { Card card; sf::Vector2f start; sf::Vector2f end; float t; float duration; };
    std::vector<DiscardAnim> animations; // animazioni in corso
    std::vector<Card> pending; // carte da aggiungere a graveyard quando animazione termina
};
