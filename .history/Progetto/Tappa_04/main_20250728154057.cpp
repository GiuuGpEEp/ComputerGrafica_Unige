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
    sf::Vector2u handAreaSize(windowSize.x, windowSize.y / 5); // ad esempio, 1/5 dell'altezza per la mano
    int numSlots = 10; // numero massimo di carte in mano

    // Calcola la larghezza massima per ogni slot
    float cardWidth = static_cast<float>(handAreaSize.x) / numSlots * 0.8f; // 80% dello slot per padding
    float cardHeight = handAreaSize.y * 0.8f; // 80% dell'altezza dell'area mano

    // Carica le carte con la dimensione calcolata
    for (int i = 0; i < initialCard; ++i) {
        Card card(textureFlipped, textureNonFlipped, detailFont);
        card.setSize(sf::Vector2f(cardWidth, cardHeight));
        // Posiziona la carta nello slot corrispondente
        float x = (i + 0.1f) * (handAreaSize.x / numSlots); // 0.1f per padding a sinistra
        float y = windowSize.y - handAreaSize.y + (handAreaSize.y - cardHeight) / 2;
        card.setPosition(sf::Vector2f(x, y));
        cards.push_back(card);
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