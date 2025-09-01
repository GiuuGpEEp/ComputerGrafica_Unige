#pragma once
#include <vector>
#include <string>
#include <optional>
#include "../../Deck/Deck.h"

class Player {
public:
    // Now store a non-owning pointer to a Deck provided by the caller.
    // This prevents accidental copies of the Deck and keeps UI/Game in sync.
    Player(const std::string& n, Deck& d);
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
    Deck* deckPtr; // non-owning pointer to shared Deck instance
    std::vector<Card> hand;
    std::vector<Card> fieldMonsters; // placeholder
    std::vector<Card> fieldSpellTraps; // placeholder
    int lifePoints = 8000; 
};
