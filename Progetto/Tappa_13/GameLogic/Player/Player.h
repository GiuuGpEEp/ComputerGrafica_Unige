#pragma once
#include <vector>
#include <string>
#include <optional>
#include "../../Deck/Deck.h"

class Player {
public:
    Player(const std::string& n, const Deck& d);
    bool canDraw() const;
    std::optional<Card> drawOne();

    const std::string& getName() const;
    int getLifePoints() const;
    void setLifePoints(int lp); // set assoluto
    void damage(int amount);    // sottrae LP
    std::vector<Card>& getHand();
    const Deck& getDeck() const;
    Deck& getDeck();
    void clearHand();

private:
    std::string name;
    Deck deck; 
    std::vector<Card> hand;
    std::vector<Card> fieldMonsters; // placeholder
    std::vector<Card> fieldSpellTraps; // placeholder
    int lifePoints = 8000; 
};
