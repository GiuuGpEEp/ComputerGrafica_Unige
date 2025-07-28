#include <SFML/Graphics.hpp>
#include "Field/Field.h"
#include "Card/Card.h"
#include <vector>
#include <iostream>

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
    for (int i = 0; i < initialCard; ++i) {
        sf::Vector2f pos(startX + i * (cardSize.x + spacing), y);
        cards.emplace_back("Carta " + std::to_string(i + 1), "Descrizione della carta " + std::to_string(i + 1), 
                           1000, 800, pos, cardSize, textureNonFlipped);
    }

    //for (int i = 0; i < initialCard; ++i) {
    //    // Ottieni la posizione e la size di uno slot della mano dal campo
    //    sf::Vector2f slotSize = calculateSlotSize(windowSize);
    //    // Rendi la carta leggermente più grande dello slot
    //    float scaleFactor = 1.1f;
    //    sf::Vector2f cardSize(slotSize.x * scaleFactor, slotSize.y * scaleFactor);
    //    sf::Vector2f cardPos(
    //        slotSize.x + (slotSize.x - cardSize.x) / 2,
    //        slotSize.y + (slotSize.y - cardSize.y) / 2
    //    );
    //    // Crea la carta (ipotizzando che il costruttore accetti posizione, size, texture, font, ecc.)
    //    cards.emplace_back(cardPos, cardSize, textureNonFlipped, textureFlipped, detailFont);
    //}

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

                    for (auto& card : cards) {
                        if (card.isClicked(mousePos)) {
                            std::cout << "Hai cliccato: " << card.getName() << std::endl;
                            // Eventuale logica: trascinamento, gioco carta, ecc.
                        }
                    }
                }    
            }
        }
        
        window.clear(sf::Color::Black);

        // Ottieni la posizione del mouse
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        field.draw(window, mousePos);
        for (auto& card : cards) card.draw(window);
        window.display();
    }
    return 0;
}