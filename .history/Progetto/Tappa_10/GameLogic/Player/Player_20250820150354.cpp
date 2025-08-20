#include "Player.h"

Player::Player(const std::string& n, const Deck& d)
    : name(n), deck(d) {}
    
    
bool canDraw() const { return deck.getSize() > 0; }    

    