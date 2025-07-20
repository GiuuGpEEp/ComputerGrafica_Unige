#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    std::cout << "Avvio del programma..." << std::endl;

    // Test 1: Creazione finestra
    std::cout << "Creando finestra..." << std::endl;
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Test Basico");
    std::cout << "Finestra creata con successo!" << std::endl;

    // Test 2: Rendering basico
    std::cout << "Inizio loop rendering..." << std::endl;
    
    int frameCount = 0;
    while(window.isOpen() && frameCount < 300){ // Limito a 300 frame per test
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                std::cout << "Richiesta chiusura finestra" << std::endl;
                window.close();
            }
        }
        
        window.clear(sf::Color::Blue);
        window.display();
        
        frameCount++;
        if (frameCount % 60 == 0) {
            std::cout << "Frame: " << frameCount << std::endl;
        }
    }
    
    std::cout << "Programma terminato correttamente dopo " << frameCount << " frame." << std::endl;
    return 0;
}