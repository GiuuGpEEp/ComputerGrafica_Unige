#include "Field/Field.h"
#include <iostream>

int main(){
    // Definisci le dimensioni della finestra
    sf::Vector2u windowSize(800, 600);
    sf::RenderWindow window(sf::VideoMode(windowSize), "Progetto Tappa 03 - Test Field");

    // Carico le texture necessarie
    sf::Texture fieldTexture("../../Progetto/resources/textures/backgroundTexture.jpg");
    sf::Texture monsterTexture("../../Progetto/resources/textures/monsterText.png");
    sf::Texture spellTrapTexture("../../Progetto/resources/textures/spellTrapTexture.png");
    sf::Texture deckTexture("../../Progetto/resources/textures/deckTexture.png");
    sf::Texture graveyardTexture("../../Progetto/resources/textures/graveTexture.png");
    sf::Texture extraDeckTexture("../../Progetto/resources/textures/ExtraDeckTexture.png");
    sf::Texture fieldSpellTexture("../../Progetto/resources/textures/fieldSpell.png");

    // Crea il campo di gioco con dimensioni dinamiche
    Field field
    (
        fieldTexture, 
        monsterTexture, 
        spellTrapTexture, 
        deckTexture, 
        graveyardTexture, 
        extraDeckTexture, 
        fieldSpellTexture, 
        windowSize
    );        

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