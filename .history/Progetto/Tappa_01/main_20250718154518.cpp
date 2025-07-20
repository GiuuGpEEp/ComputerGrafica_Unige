#include "Card.h"
#include <vector>

int main() {

    // Creo le prime carte per fare dei test
    std::vector<Card> cards;
    cards.emplace_back("Drago Bianco", "Drago pazzo pazzissimo", 3000, 2500, sf::Vector2f(50, 50));
    cards.emplace_back("Mago Nero", "Mago altamente razzista", 2500, 2000, sf::Vector2f(200, 50));

    // Inizializzo la finestra di rendering
    sf::RenderWindow window(sf::VideoMode({800u, 600u}), "Progetto Tappa 01");

    while(window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                window.close();
                break;
            }

            if (const auto *mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Mouse::Button but = mouseButton->button;
                if(but == sf::Mouse::Button::Left) {
                    sf::Vector2i mousePos = mouseButton->position;
                    for (auto& card : cards) {
                        if(card)
            }
        }
    }
}