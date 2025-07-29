#include <SFML/Graphics.hpp>
#include "Field/Field.h"
#include "Card/Card.h"
#include <vector>
#include <iostream>
#define P1 1
#define P2 2

int main(){
    // Definisci le dimensioni della finestra
    sf::Vector2u windowSize(1920, 1080);
    sf::RenderWindow window(sf::VideoMode(windowSize), "Progetto Tappa 04 - Field + Card");

    // Carico le texture necessarie per il campo di gioco
    sf::Texture fieldTexture("../../Progetto/resources/backgroundTexture.jpg");
    sf::Texture monsterTexture("../../Progetto/resources/monsterText.png");
    sf::Texture spellTrapTexture("../../Progetto/resources/spellTrapTexture.png");
    sf::Texture deckTexture("../../Progetto/resources/deckTexture.png");
    sf::Texture graveyardTexture("../../Progetto/resources/graveTexture.png");
    sf::Texture extraDeckTexture("../../Progetto/resources/ExtraDeckTexture.png");
    sf::Texture fieldSpellTexture("../../Progetto/resources/fieldSpell.png");

    // Carico le texture necessarie per le carte
    sf::Texture textureFlipped(("../../Progetto/resources/Texture1.png"));
    sf::Texture textureNonFlipped(("../../Progetto/resources/CardNotSet.jpg"));

    // Carico il font per le etichette delle carte
    sf::Font detailFont;
    if (!detailFont.openFromFile("../../Progetto/resources/ITCKabelStdDemi.TTF")) {

        if (!detailFont.openFromFile("C:/Windows/Fonts/calibri.ttf")) {
            std::cerr << "ERRORE: Impossibile caricare nessun font!" << std::endl;
        }
    }

    // Crea il campo di gioco con dimensioni dinamiche
    Field field(
        fieldTexture, 
        monsterTexture, 
        spellTrapTexture, 
        deckTexture, 
        graveyardTexture, 
        extraDeckTexture, 
        fieldSpellTexture, 
        windowSize
    );        

    //Creo il deck 
    std::vector<Card> deck;
    deck.reserve(30);
    sf::Vector2f deckSlotPos = field.getSlotPosition(Slot::Type::Deck, P1); 
    
    //Visualizzo il deck (ovviemente non per intero, ma solo un paio di carte a livello "simbolico")
    int visibleCard = std::min



    // Creo le carte in mano
    std::vector<Card> cards;
    cards.reserve(10); 
    const int initialCard = 5;

    //Calcolo la dimensione della carta rendendola un po' più grande delle dimensioni degli slot e prendo i parametri di posizione e spaziatura
    sf::Vector2f slotSize = calculateSlotSize(windowSize);
    float scaleFactor = 1.1f;
    sf::Vector2f cardSize(slotSize.x * scaleFactor, slotSize.y * scaleFactor);
    float spacing = 15.f;
    float totalHandWidth = initialCard * cardSize.x + (initialCard - 1) * spacing;
    float startX = (windowSize.x - totalHandWidth) / 2.f;
    // Ripristino la posizione originale delle carte
    float y = windowSize.y - cardSize.y - 10.f; 

    // Creo le carte e le posiziono nella mano
    for (int i = 0; i < initialCard-1; ++i) {
        sf::Vector2f pos(startX + i * (cardSize.x + spacing), y);
        cards.emplace_back("Carta " + std::to_string(i + 1), "Descrizione della carta " + std::to_string(i + 1), 
                           1000, 800, pos, cardSize, textureNonFlipped);
    }
    sf::Vector2f pos(startX + 4 * (cardSize.x + spacing), y);
        cards.emplace_back("Drago Bianco", "Questo drago leggendario e' una potente macchina distruttrice. Virtualmente invincibile, sono in pochi ad aver fronteggiato questa creatura ed essere sopravvissuti per raccontarlo.", 
                           3000, 2500, pos, cardSize, textureNonFlipped);
   
    std::optional<size_t> selectedCardIndex; // Indice della carta selezionata
    float scrollOffset = 0.f; //Offset per lo scroll del testo dei dettagli della carta

    while(window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                std::cout << "Richiesta di chiusura..." << std::endl;
                window.close();
            }

            if (const auto *mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Mouse::Button but = mouseButton->button;
                if(but == sf::Mouse::Button::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                    bool cardClicked = false;
                    for (size_t i = 0; i < cards.size(); ++i) {
                        if (cards[i].isClicked(mousePos)) {
                            // Se è una carta diversa da quella già selezionata, resetta lo scroll
                            if (selectedCardIndex.has_value() && selectedCardIndex.value() != i) {
                                scrollOffset = 0.f; // Reset dello scroll
                            }

                            selectedCardIndex = i; // Aggiorna l'indice della carta selezionata
                            cardClicked = true;
                            std::cout << "Hai cliccato: " << cards[i].getName() << std::endl;
                            break;
                        }
                    }

                     // Se ho cliccato fuori dalle carte, deseleziona
                    if (!cardClicked) {
                        selectedCardIndex.reset();
                        scrollOffset = 0.f; // Resettando lo scroll quando seleziono fuori (o un'altra carta) ritorno in cima alla view
                    }
                }    
            }

            // Gestione tasto ESC per deselezionare
            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    selectedCardIndex.reset();
                    scrollOffset = 0.f; // Resetta lo scroll anche con ESC
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
        
        window.clear(sf::Color::Black);

        // Ottieni la posizione del mouse
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        
        // Disegna il campo di gioco e le carte
        field.draw(window, mousePos);
        for (auto& card : cards) card.draw(window);

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
        
        window.display();
    }
    return 0;
}