#include "Card/Card.h"
#include <vector>

int main() {
    std::vector<Card> cards;
    cards.emplace_back("Drago Bianco", "Drago pazzo pazzissimo", 3000, 2500, sf::Vector2f(50, 50));
    cards.emplace_back("Mago Nero", "Mago altamente razzista", 2500, 2000, sf::Vector2f(200, 50));

    sf::RenderWindow window(sf::VideoMode(800, 600), "Progetto Tappa 01");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = {event.mouseButton.x, event.mouseButton.y};
                    for (auto& card : cards) {
                        if (card.isClicked(mousePos)) {
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

        window.clear(sf::Color::Black);
        for (auto& card : cards) {
            card.draw(window);
        }
        window.display();
    }

    return 0;
}
