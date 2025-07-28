#include <SFML/Graphics.hpp>
#include "Field/Field.h"
#include "Card/Card.h"
#include <vector>
#include <iostream>

int main(){
    // Definisci le dimensioni della finestra
    sf::Vector2u windowSize(800, 600);
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
    for (int i = 0; i < initialCard; ++i) {
        // Ottieni la posizione e la size di uno slot della mano dal campo
        sf::FloatRect slotSize = calculateSlotSize(windowSize);
        // Rendi la carta leggermente più grande dello slot
        float scaleFactor = 1.1f;
        sf::Vector2f cardSize(slotRect.width * scaleFactor, slotRect.height * scaleFactor);
        sf::Vector2f cardPos(
            slotRect.left + (slotRect.width - cardSize.x) / 2,
            slotRect.top + (slotRect.height - cardSize.y) / 2
        );
        // Crea la carta (ipotizzando che il costruttore accetti posizione, size, texture, font, ecc.)
        cards.emplace_back(cardPos, cardSize, textureNonFlipped, textureFlipped, detailFont);
    }





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