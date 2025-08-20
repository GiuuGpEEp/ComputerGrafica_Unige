#include "Player.h"

Player::Player(const std::string& n, const Deck& d)
    : name(n), deck(d) {}
    
    
bool Player::canDraw() const { 
    return deck.getSize() > 0; 
}

std::optional<Card> Player::drawOne(){
    if(!canDraw()) return std::nullopt;
    Card c = deck.drawCard();
    hand.push_back(c);
    return c;
}    

const std::string& Player::getName() const { 
    return name; 
}

int Player::getLifePoints() const {
    return lifePoints;
}

std