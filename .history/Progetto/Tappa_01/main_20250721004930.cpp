#include <SFML/Graphics.hpp>
#include <iostream>
#include "Card/Card.h"

int main() {
    std::cout << "Avvio del programma..." << std::endl;

    // Test 1: Creazione finestra
    std::cout << "Creando finestra..." << std::endl;
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Test Carte");
    std::cout << "Finestra creata con successo!" << std::endl;

    // Test 2: Creazione carte
    std::cout << "Creando prima carta..." << std::endl;
    try {
        Card card1("Drago", "Test", 1000, 500, sf::Vector2f(50, 50));
        std::cout << "Prima carta creata con successo!" << std::endl;
        
        std::cout << "Creando seconda carta..." << std::endl;
        Card card2("Mago", "Test2", 800, 300, sf::Vector2f(200, 50));
        std::cout << "Seconda carta creata con successo!" << std::endl;

        // Test 3: Rendering con carte
        std::cout << "Inizio loop rendering con carte..." << std::endl;
        
        int frameCount = 0;
        while(window.isOpen() && frameCount < 180){ // 3 secondi a 60fps
            while (const std::optional event = window.pollEvent()){
                if (event->is<sf::Event::Closed>()) {
                    std::cout << "Richiesta chiusura finestra" << std::endl;
                    window.close();
                }
            }
            
            window.clear(sf::Color::Black);
            
            // Test rendering delle carte
            try {
                card1.draw(window);
                card2.draw(window);
            } catch (const std::exception& e) {
                std::cout << "ERRORE durante rendering: " << e.what() << std::endl;
                break;
            }
            
            window.display();
            
            frameCount++;
            if (frameCount % 60 == 0) {
                std::cout << "Frame: " << frameCount << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cout << "ERRORE durante creazione carte: " << e.what() << std::endl;
        return -1;
    }
    
    std::cout << "Programma terminato correttamente!" << std::endl;
    return 0;
}