#include "Card/Card.h"
#include <vector>

int main() {

    // DEBUG: Test caricamento font
    sf::Font testFont;
    if (!testFont.loadFromFile("resources/ITCKabelStdDemi.TTF")) {
        std::cout << "ERRORE: Font non trovato nel percorso 'resources/ITCKabelStdDemi.TTF'" << std::endl;
        
        // Prova un percorso alternativo
        if (!testFont.loadFromFile("../../resources/ITCKabelStdDemi.TTF")) {
            std::cout << "ERRORE: Font non trovato neanche nel percorso '../../resources/ITCKabelStdDemi.TTF'" << std::endl;
            std::cout << "Usando font di sistema predefinito..." << std::endl;
        } else {
            std::cout << "SUCCESS: Font caricato dal percorso '../../resources/ITCKabelStdDemi.TTF'" << std::endl;
        }
    } else {
        std::cout << "SUCCESS: Font caricato dal percorso 'resources/ITCKabelStdDemi.TTF'" << std::endl;
    }

    // Inizializzo la finestra di rendering
    sf::RenderWindow window(sf::VideoMode({800, 600}), "DEBUG - Test Font");

    // Creo un testo di test
    sf::Text debugText;
    debugText.setFont(testFont);
    debugText.setString("TESTO DI DEBUG\nFont caricato correttamente!");
    debugText.setCharacterSize(24);
    debugText.setFillColor(sf::Color::White);
    debugText.setPosition(50, 50);

    while(window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        
        window.clear(sf::Color::Black);
        window.draw(debugText);
        window.display();
    }
    return 0;
}