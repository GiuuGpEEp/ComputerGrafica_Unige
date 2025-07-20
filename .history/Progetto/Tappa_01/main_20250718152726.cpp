#include "Card.h"
#include <vector>

int main() {

    // Create a vector of Card objects
    std::vector<Card> cards;
    cards.emplace_back("Drago Bianco", "Drago pazzo pazzissimo", 10, 5, sf::Vector2f(50, 50));
    cards.emplace_back("Mago Nero", "Mago ", 8, 7, sf::Vector2f(200, 50));


}