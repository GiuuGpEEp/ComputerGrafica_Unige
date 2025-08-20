#pragma once
#include <vector>
#include <string>
#include <optional>
#include "../Deck/Deck.h"

class Player {
public:
    Player(const std::string& n, const Deck& d);
    bool canDraw() const;
    std::optional<Card> drawOne();

    const std::string& getName() const { return name; }
    int getLifePoints() const { return lifePoints; }
    const std::vector<Card>& getHand() const { return hand; }
    std::vector<Card>& getHand() { return hand; }
    Deck& getDeck() { return deck; }
    const Deck& getDeck() const { return deck; }

private:
    std::string name;
    Deck deck; 
    std::vector<Card> hand;
    std::vector<Card> fieldMonsters; // placeholder
    std::vector<Card> fieldSpellTraps; // placeholder
    int lifePoints = 8000; 
};
