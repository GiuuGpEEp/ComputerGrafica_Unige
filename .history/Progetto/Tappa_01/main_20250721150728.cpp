#include "Card/Card.h"
#include <vector>
#include <optional>

int main() {

    // Creo le prime carte per fare dei test
    std::vector<Card> cards;
    cards.reserve(10); // Riservo spazio per evitare riallocazioni che causavano crash
    cards.emplace_back("Drago Bianco", "Drago pazzo pazzissimo", 3000, 2500, sf::Vector2f(50, 50));
    cards.emplace_back("Mago Nero", "Mago altamente razzista", 2500, 2000, sf::Vector2f(200, 50));

    // Inizializzo la finestra di rendering
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Progetto Tappa 01");

    // Variabili per gestire la carta selezionata
    std::optional<size_t> selectedCardIndex;
    sf::Font detailFont;
    
    // Carico il font per i dettagli
    if (!detailFont.openFromFile("ITCKabelStdDemi.TTF")) {
        detailFont.openFromFile("C:/Windows/Fonts/calibri.ttf");
    }

    while(window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto *mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Mouse::Button but = mouseButton->button;
                if(but == sf::Mouse::Button::Left) {
                    sf::Vector2i mousePos = mouseButton->position;
                    
                    // Controlla se ho cliccato su una carta
                    bool cardClicked = false;
                    for (size_t i = 0; i < cards.size(); ++i) {
                        if(cards[i].isClicked(mousePos)){
                            selectedCardIndex = i; // Seleziona questa carta
                            cardClicked = true;
                            break;
                        }
                    }
                    
                    // Se ho cliccato fuori dalle carte, deseleziona
                    if (!cardClicked) {
                        selectedCardIndex.reset();
                    }
                }
            }
            
            // Gestione tasto ESC per deselezionare
            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    selectedCardIndex.reset();
                }
            }
        }
        
        window.clear(sf::Color::Black);
        
        // Disegna le carte
        for (auto& card : cards) {
            card.draw(window);
        }
        
        // Se c'è una carta selezionata, mostra i dettagli
        if (selectedCardIndex.has_value()) {
            const Card& selectedCard = cards[selectedCardIndex.value()];
            
            // Crea un pannello per i dettagli
            sf::RectangleShape detailPanel({300.f, 200.f});
            detailPanel.setFillColor(sf::Color(0, 0, 0, 200)); // Nero semi-trasparente
            detailPanel.setOutlineColor(sf::Color::White);
            detailPanel.setOutlineThickness(2.f);
            detailPanel.setPosition({400.f, 150.f});
            
            // Testo dei dettagli
            auto [atk, def] = selectedCard.getValues();
            std::string detailText = "DETTAGLI CARTA\n\n";
            detailText += "Nome: " + selectedCard.getName() + "\n\n";
            detailText += "Descrizione:\n" + selectedCard.getDescription() + "\n\n";
            detailText += "ATK: " + std::to_string(atk) + "\n";
            detailText += "DEF: " + std::to_string(def) + "\n\n";
            detailText += "Premi ESC per chiudere";
            
            sf::Text detailTextObj{detailFont, detailText, 14};
            detailTextObj.setFillColor(sf::Color::White);
            detailTextObj.setPosition({410.f, 160.f});
            
            // Disegna il pannello e il testo
            window.draw(detailPanel);
            window.draw(detailTextObj);
        }
        
        window.display();
    }
    return 0;
}