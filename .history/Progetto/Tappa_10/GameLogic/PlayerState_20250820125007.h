#pragma once
#include <vector>
#include "../Deck/Deck.h"
#include "../Card/Card.h"

struct PlayerState {
    int id = 0;
    int life = 8000;
    Deck* deck = nullptr;
    std::vector<Card>* hand = nullptr;
    std::vector<Card>* fieldCards = nullptr;
};