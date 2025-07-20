#include "Card/Card.h"
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
                        if(card.isClicked(mousePos)){
                            std::cout << "╔══════════════════════════════════════════════════════════╗" << std::endl;
                            std::cout << "  Carta: " << card.getName() << std::endl;
                            std::cout << "╟──────────────────────────────────────────────────────────╢" << std::endl;
                            std::cout << "  Descrizione: " << card.getDescription() << std::endl;
                            auto [atk, def] = card.getValues();
                            std::cout << "  ATK: " << atk << "   |   DEF: " << def << std::endl;
                            std::cout << "╚══════════════════════════════════════════════════════════╝" << std::endl;
                        }
                    }
                }
            }
        }
        window.clear(sf::Color::White);
        for (auto& card : cards) {
            card.draw(window);
        }
        window.display();
    }
    return 0;
}