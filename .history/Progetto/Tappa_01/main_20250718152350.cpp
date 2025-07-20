#include "Card.h"
#include <vector>

int main() {

    // Create a vector of Card objects
    std::vector<Card> cards;
    cards.push_back("Card1", "Description1", 10, 5, sf::Vector2f(50, 50));
    cards.push_back("Card2", "Description2", 8, 7, sf::Vector2f(200, 50));


}