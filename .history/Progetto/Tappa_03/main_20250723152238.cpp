#include "Field/Field.h"
#include <iostream>

int main(){
    sf::RenderWindow window(sf::VideoMode({800,600}), "Progetto Tappa 03 - Test Field");

    // Carico le texture necessarie
    sf::Texture fieldTexture("../../Progetto/resources/backgroundTexture.jgp");
    sf::Texture slotsTexture("../../Progetto/resources/slotTexture.png");

    // Crea il campo di gioco
    Field field(fieldTexture, slotsTexture);

    while(window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                std::cout << "Richiesta di chiusura..." << std::endl;
                window.close();
            }
        }
        
        window.clear(sf::Color::Black);

        // Ottieni la posizione del mouse
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        field.draw(window, mousePos);
        
        window.display();
    }
    return 0;
}