#include "Card/Card.h"
#include <vector>

int main() {

    // Carico la texture della carta
    sf::Texture textureFlipped(("../../Progetto/resources/Texture1.png"));
    sf::Texture textureNonFlipped(("../../Progetto/resources/CardNotSet.jpg"));

    // Creo le prime carte per fare dei test
    std::vector<Card> cards;
    cards.reserve(10); // Riservo spazio per evitare riallocazioni che causavano crash
    cards.emplace_back("Drago Bianco", "Questo drago leggendario è una potente macchina distruttrice. Virtualmente invincibile, sono in pochi ad aver fronteggiato questa creatura ed essere sopravvissuti per raccontarlo.", 3000, 2500, sf::Vector2f(50, 50), textureFlipped);
    cards.emplace_back("Mago Nero", "Mago altamente razzista", 2500, 2000, sf::Vector2f(200, 50), textureNonFlipped);

    // Inizializzo la finestra di rendering
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Progetto Tappa 02");
      
    // Variabili per gestire la carta selezionata
    std::optional<size_t> selectedCardIndex; // Indice della carta selezionata
    
    // Carico il font per le etichette delle carte
    sf::Font detailFont;
    if (!detailFont.openFromFile("../../Progetto/resources/ITCKabelStdDemi.TTF")) {

        if (!detailFont.openFromFile("C:/Windows/Fonts/calibri.ttf")) {
            std::cerr << "ERRORE: Impossibile caricare nessun font!" << std::endl;
        }
    }

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
                    
                    //Gestisco il click su una carta
                    bool cardClicked = false;
                    for (size_t i = 0; i < cards.size(); ++i) {
                        if(cards[i].isClicked(mousePos)){
                            selectedCardIndex = i; // Salvo l'indice della carta selezionata
                            cardClicked = true;
                            std::cout << "Carta cliccata: " << cards[i].getName() << std::endl;
                            break; // Esco dal ciclo una volta trovata la carta cliccata
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
                    std::cout << "Carta deselezionata con ESC" << std::endl;
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