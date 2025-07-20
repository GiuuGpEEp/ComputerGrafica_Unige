#include "Card/Card.h"
#include <vector>

int main() {

    // DEBUG: Test caricamento font
    sf::Font testFont;
    bool fontLoaded = false;
    
    std::cout << "=== DEBUG FONT LOADING ===" << std::endl;
    
    if (!testFont.openFromFile("resources/ITCKabelStdDemi.TTF")) {
        std::cout << "ERRORE: Font non trovato nel percorso 'resources/ITCKabelStdDemi.TTF'" << std::endl;
        
        // Prova un percorso alternativo
        if (!testFont.openFromFile("../../resources/ITCKabelStdDemi.TTF")) {
            std::cout << "ERRORE: Font non trovato neanche nel percorso '../../resources/ITCKabelStdDemi.TTF'" << std::endl;
            
            // Prova altri percorsi possibili
            if (!testFont.openFromFile("../resources/ITCKabelStdDemi.TTF")) {
                std::cout << "ERRORE: Font non trovato neanche nel percorso '../resources/ITCKabelStdDemi.TTF'" << std::endl;
                std::cout << "Provo con il font di sistema..." << std::endl;
                fontLoaded = false;
            } else {
                std::cout << "SUCCESS: Font caricato dal percorso '../resources/ITCKabelStdDemi.TTF'" << std::endl;
                fontLoaded = true;
            }
        } else {
            std::cout << "SUCCESS: Font caricato dal percorso '../../resources/ITCKabelStdDemi.TTF'" << std::endl;
            fontLoaded = true;
        }
    } else {
        std::cout << "SUCCESS: Font caricato dal percorso 'resources/ITCKabelStdDemi.TTF'" << std::endl;
        fontLoaded = true;
    }
    
    std::cout << "Font caricato: " << (fontLoaded ? "SI" : "NO") << std::endl;
    std::cout << "=========================" << std::endl;

    // Inizializzo la finestra di rendering
    sf::RenderWindow window(sf::VideoMode({800, 600}), "DEBUG - Test Font");

    // Creo un testo di test
    sf::Text debugText{testFont, "TEST", 48};
    debugText.setFillColor(sf::Color::White);
    debugText.setPosition({50.f, 50.f});
    
    // Creo anche un rettangolo di riferimento per vedere se il rendering funziona
    sf::RectangleShape testRect{{200.f, 100.f}};
    testRect.setFillColor(sf::Color::Red);
    testRect.setPosition({50.f, 150.f});

    while(window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        
        window.clear(sf::Color::Black);
        window.draw(testRect);  // Rettangolo rosso di riferimento
        window.draw(debugText);
        window.display();
    }
    return 0;
}