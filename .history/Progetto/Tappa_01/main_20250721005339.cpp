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
    
    // Commento temporaneamente la seconda carta per test
    // cards.emplace_back("Mago Nero", "Mago altamente razzista", 2500, 2000, sf::Vector2f(200, 50));
    // std::cout << "Seconda carta creata!" << std::endl;

    std::cout << "Numero di carte nel vector: " << cards.size() << std::endl;
    std::cout << "Inizio loop principale..." << std::endl;

    int frameCount = 0;
    while(window.isOpen()){
        frameCount++;
        if (frameCount == 1) {
            std::cout << "Primo frame..." << std::endl;
        }
        
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                std::cout << "Richiesta di chiusura..." << std::endl;
                window.close();
            }

            if (const auto *mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Mouse::Button but = mouseButton->button;
                if(but == sf::Mouse::Button::Left) {
                    sf::Vector2i mousePos = mouseButton->position;
                    std::cout << "Click a posizione: " << mousePos.x << ", " << mousePos.y << std::endl;
                    
                    for (size_t i = 0; i < cards.size(); ++i) {
                        if(cards[i].isClicked(mousePos)){
                            std::cout << "╔══════════════════════════════════════════════════════════╗" << std::endl;
                            std::cout << "  Carta: " << cards[i].getName() << std::endl;
                            std::cout << "╟──────────────────────────────────────────────────────────╢" << std::endl;
                            std::cout << "  Descrizione: " << cards[i].getDescription() << std::endl;
                            auto [atk, def] = cards[i].getValues();
                            std::cout << "  ATK: " << atk << "   |   DEF: " << def << std::endl;
                            std::cout << "╚══════════════════════════════════════════════════════════╝" << std::endl;
                        }
                    }
                }
            }
        }
        
        if (frameCount == 1) {
            std::cout << "Inizio clear..." << std::endl;
        }
        window.clear(sf::Color::Black);
        
        if (frameCount == 1) {
            std::cout << "Inizio draw delle " << cards.size() << " carte..." << std::endl;
        }
        for (size_t i = 0; i < cards.size(); ++i) {
            if (frameCount == 1) {
                std::cout << "Drawing carta " << i << std::endl;
            }
            cards[i].draw(window);
        }
        
        if (frameCount == 1) {
            std::cout << "Inizio display..." << std::endl;
        }
        window.display();
        
        if (frameCount == 1) {
            std::cout << "Primo frame completato!" << std::endl;
        }
        if (frameCount % 60 == 0) {
            std::cout << "Frame: " << frameCount << std::endl;
        }
    }
    
    std::cout << "Uscito dal loop, distruggendo carte..." << std::endl;
    
    std::cout << "Programma terminato correttamente!" << std::endl;
    return 0;
}