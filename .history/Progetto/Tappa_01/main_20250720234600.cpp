#include "Card/Card.h"
#include <vector>

int main() {
    std::vector<Card> cards;
    cards.emplace_back("Drago Bianco", "Drago pazzo pazzissimo", 3000, 2500, sf::Vector2f{50, 50});
    cards.emplace_back("Mago Nero", "Mago altamente razzista", 2500, 2000, sf::Vector2f{200, 50});

    sf::RenderWindow window(sf::VideoMode{800u, 600u}, "Progetto Tappa 01");

    while(window.isOpen()) {
        // Versione con poll() senza parametri
        if (window.pollEvent()) {
            if (window.hasEvent()) {
                sf::Event event = window.getEvent();
                
                if (event.type == sf::Event::Closed) {
                    window.close();
                }

                if (event.type == sf::Event::MouseButtonPressed) {
                    if(event.mouseButton.button == sf::Mouse::Left) {
                        sf::Vector2i mousePos(event.mouseButton.x, event.mouseButton.y);
                        for (auto& card : cards) {
                            if(card.isClicked(mousePos)){
                                std::cout << "╔══════════════════════════════════════════════════════════╗\n";
                                std::cout << "  Carta: " << card.getName() << "\n";
                                std::cout << "╟──────────────────────────────────────────────────────────╢\n";
                                std::cout << "  Descrizione: " << card.getDescription() << "\n";
                                auto [atk, def] = card.getValues();
                                std::cout << "  ATK: " << atk << "   |   DEF: " << def << "\n";
                                std::cout << "╚══════════════════════════════════════════════════════════╝\n";
                            }
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