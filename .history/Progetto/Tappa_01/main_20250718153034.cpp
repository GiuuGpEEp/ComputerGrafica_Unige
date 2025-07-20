#include "Card.h"
#include <vector>

int main() {

    // Creo le prime carte per fare dei test
    std::vector<Card> cards;
    cards.emplace_back("Drago Bianco", "Drago pazzo pazzissimo", 3000, 2500, sf::Vector2f(50, 50));
    cards.emplace_back("Mago Nero", "Mago altamente razzista", 8, 7, sf::Vector2f(200, 50));

    // Inizializzo la finestra di rendering
    sf::RenderWindow window(sf::VideoMode({800u, 600u}), "Card Game");


}