#include "Card/Card.h"
#include "../resources/aux.cpp"
#include <vector>

int main() {

    // Carico la texture della carta
    sf::Texture textureFlipped(("../../Progetto/resources/Texture1.png"));
    sf::Texture textureNonFlipped(("../../Progetto/resources/CardNotSet.jpg"));

    // Creo le prime carte per fare dei test
    std::vector<Card> cards;
    cards.reserve(10); // Riservo spazio per evitare riallocazioni che causavano crash
    cards.emplace_back("Drago Bianco", "Questo drago leggendario è una potente macchina distruttrice. Virtualmente invincibile, sono in pochi ad aver fronteggiato questa creatura ed essere sopravvissuti per raccontarlo."
        , 3000, 2500, sf::Vector2f(50, 50), textureFlipped);
    cards.emplace_back("Mago Nero", "Il più potente tra i maghi per abilità offensive e difensive.", 2500, 2000, sf::Vector2f(200, 50), textureNonFlipped);

    // Inizializzo la finestra di rendering
    sf::RenderWindow window(sf::VideoMode({1920, 600}), "Progetto Tappa 02");
      
    // Variabili per gestire la carta selezionata
    std::optional<size_t> selectedCardIndex; // Indice della carta selezionata
    
    // Carico il font per le etichette delle carte
    sf::Font detailFont;
    if (!detailFont.openFromFile("../../Progetto/resources/ITCKabelStdDemi.TTF")) {

        if (!detailFont.openFromFile("C:/Windows/Fonts/calibri.ttf")) {
            std::cerr << "ERRORE: Impossibile caricare nessun font!" << std::endl;
        }
    }
    
    float scrollOffset = 0.f; //Offset per lo scroll del testo dei dettagli della carta

    // Inizio del ciclo principale 
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

             // Scroll mouse
            if (const auto* mouseScroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (selectedCardIndex.has_value()) {
                    scrollOffset -= mouseScroll->delta * 20.f;
                    scrollOffset = std::max(0.f, scrollOffset); // niente scroll negativo
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

        // Disegno le carte
        for (size_t i = 0; i < cards.size(); ++i) {
            if (frameCount == 1) {
                std::cout << "Drawing carta " << i << std::endl;
            }
            cards[i].draw(window);
        }

        // Se c'è una carta selezionata, mostra i dettagli
        if (selectedCardIndex.has_value()) {
            const Card& selectedCard = cards[selectedCardIndex.value()];

            // Valori della carta
            auto [atk, def] = selectedCard.getValues();

            // Pannello dei dettagli
            sf::Vector2f panelPos{400.f, 150.f};
            sf::Vector2f panelSize{300.f, 200.f};
            sf::RectangleShape detailPanel(panelSize);
            detailPanel.setFillColor(sf::Color(0, 0, 0, 200));
            detailPanel.setOutlineColor(sf::Color::White);
            detailPanel.setOutlineThickness(2.f);
            detailPanel.setPosition(panelPos);

            // Creo la stringa con il testo principale 
            std::string detailRaw = "DETTAGLI CARTA\n\n";
            detailRaw += "Nome: " + selectedCard.getName() + "\n\n";
            detailRaw += "Descrizione:\n" + selectedCard.getDescription() + "\n\n";
            detailRaw += "ATK: " + std::to_string(atk) + "\n";
            detailRaw += "DEF: " + std::to_string(def);

            // Applica word wrapping al testo principale
            float textMaxWidth = panelSize.x - 40.f; // Margine di 20px per lato
            std::string wrappedText = wrapText(detailRaw, detailFont, 14, textMaxWidth);

            // Partendo dalla stringa originale, dopo averne fatto il wrapping, creo l'effettivo oggetto Text (il testo scrollabile effettivo)
            sf::Text detailTextObj(detailFont, wrappedText, 14);
            detailTextObj.setFillColor(sf::Color::White);
            detailTextObj.setPosition({panelPos.x + 10.f, panelPos.y + 10.f - scrollOffset});

            // Crea il testo ESC fisso in fondo al pannello
            sf::Text escTextObj(detailFont, "Premi ESC per chiudere", 12);
            escTextObj.setFillColor(sf::Color::Yellow);
            escTextObj.setPosition({panelPos.x + 10.f, panelPos.y + panelSize.y - 25.f});

            // Disegna prima il pannello 
            window.draw(detailPanel);

            // Salva la view corrente
            sf::View originalView = window.getView();
            
            // Crea una view limitata al pannello per il clipping del testo scrollabile
            // Riduciamo l'altezza per lasciare spazio al testo ESC
            sf::View textView;
            textView.setSize(sf::Vector2f(panelSize.x - 20.f, panelSize.y - 50.f)); // Più spazio in basso per ESC
            textView.setCenter(sf::Vector2f(panelPos.x + panelSize.x/2.f, panelPos.y + (panelSize.y - 50.f)/2.f + 10.f));
            textView.setViewport(sf::FloatRect(
                sf::Vector2f((panelPos.x + 10.f) / window.getSize().x, (panelPos.y + 10.f) / window.getSize().y),
                sf::Vector2f((panelSize.x - 20.f) / window.getSize().x, (panelSize.y - 50.f) / window.getSize().y)
            ));

            // Imposta la view per il testo scrollabile e disegnalo
            window.setView(textView);
            window.draw(detailTextObj);
            
            // Ripristina la view originale per disegnare il testo ESC fisso
            window.setView(originalView);
            window.draw(escTextObj);

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