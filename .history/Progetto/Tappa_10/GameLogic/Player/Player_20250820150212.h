#pragma once
#include <vector>
#include <string>
#include <optional>
#include "../Deck/Deck.h"

class Player {
    

    Player(const std::string& n, const Deck& d): name(n), deck(d) {}

    bool canDraw() const { return deck.getSize() > 0; }

    std::optional<Card> drawOne(){
        if(!canDraw()) return std::nullopt;
        Card c = deck.drawCard();
        hand.push_back(c);
        return c;
    }
};
