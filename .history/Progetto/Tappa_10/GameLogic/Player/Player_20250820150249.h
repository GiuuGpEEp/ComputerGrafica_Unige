#pragma once
#include <vector>
#include <string>
#include <optional>
#include "../Deck/Deck.h"

class Player {
    
    public:
        Player(const std::string& n, const Deck& d): name(n), deck(d) {}

        bool canDraw() const { return deck.getSize() > 0; }

        std::optional<Card> drawOne(){
            if(!canDraw()) return std::nullopt;
            Card c = deck.drawCard();
            hand.push_back(c);
            return c;
        }
    private:
        std::string name;
        Deck deck; 
        std::vector<Card> hand;
        std::vector<Card> fieldMonsters; // placeholder
        std::vector<Card> fieldSpellTraps; // placeholder
        int lifePoints = 8000; 
        
};
