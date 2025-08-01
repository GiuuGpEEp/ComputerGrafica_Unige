#include <SFML/Graphics.hpp>
#include <iostream>
#include "Card/Card.h"
#include <vector>

int main() {
    std::cout << "Avvio del programma..." << std::endl;

    // Inizializzo la finestra di rendering
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Progetto Tappa 01");
    std::cout << "Finestra creata!" << std::endl;

    // Creo le carte direttamente nel vector usando emplace_back
    std::vector<Card> cards;
    std::cout << "Creando carte..." << std::endl;
    
    cards.emplace_back("Drago Bianco", "Drago pazzo pazzissimo", 3000, 2500, sf::Vector2f(50, 50));
    std::cout << "Prima carta creata!" << std::endl;
    
    cards.emplace_back("Mago Nero", "Mago altamente razzista", 2500, 2000, sf::Vector2f(200, 50));
    std::cout << "Seconda carta creata!" << std::endl;

    std::cout << "Inizio loop principale..." << std::endl;

    while(window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                window.close();
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
        
        window.clear(sf::Color::Black);
        for (auto& card : cards) {
            card.draw(window);
        }
        window.display();
    }
    
    std::cout << "Programma terminato correttamente!" << std::endl;
    return 0;
}