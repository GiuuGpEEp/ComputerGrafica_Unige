#include "Player.h"

Player::Player(const std::string& n, Deck& d)
    : name(n), deckPtr(&d) {}
    
    
bool Player::canDraw() const { 
    return deckPtr && deckPtr->getSize() > 0; 
}

std::optional<Card> Player::drawOne(){
    if(!canDraw()) return std::nullopt;
    Card c = deckPtr->drawCard();
    hand.push_back(c);
    return c;
}    

const std::string& Player::getName() const { 
    return name; 
}

int Player::getLifePoints() const {
    return lifePoints;
}

void Player::setLifePoints(int lp){
    lifePoints = std::max(0, lp);
}

void Player::damage(int amount){
    if(amount <= 0) return;
    lifePoints = std::max(0, lifePoints - amount);
}

std::vector<Card>& Player::getHand(){
    return hand;
}

const Deck& Player::getDeck() const {
    if(deckPtr) return *deckPtr;
    static Deck empty;
    return empty;
}

Deck& Player::getDeck(){
    if(deckPtr) return *deckPtr;
    static Deck empty;
    return empty;
}

void Player::clearHand(){
    hand.clear();
}

