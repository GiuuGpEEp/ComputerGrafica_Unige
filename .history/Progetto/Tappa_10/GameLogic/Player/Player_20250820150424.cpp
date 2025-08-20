#include "Player.h"

Player::Player(const std::string& n, const Deck& d)
    : name(n), deck(d) {}
    
    
bool PlacanDraw() const { 
    return deck.getSize() > 0; 
}    

std::optional<Card> drawOne(){
    if(!canDraw()) return std::nullopt;
    Card c = deck.drawCard();
    hand.push_back(c);
    return c;
}    