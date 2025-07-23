#include "Field/Field.h"

int main(){
    sf::RenderWindow window(sf::VideoMode({800,600}), "Progetto Tappa 03 - Test Field");

    // Carico le texture necessarie
    sf::Texture fieldTexture("../../Progetto/resources/fieldTexture.png");
    sf::Texture monsterTexture("../../Progetto/resources/monsterTexture.png");
    sf::Texture spellTexture("../../Progetto/resources/spellTexture.png");
    sf::Texture deckTexture("../../Progetto/resources/deckTexture.png");
    sf::Texture graveTexture("../../Progetto/resources/graveTexture.png");
    sf::Texture extraTexture("../../Progetto/resources/extraTexture.png");
    sf::Texture fieldSpellTexture("../../Progetto/resources/fieldSpellTexture.png");

    Field field(fieldTexture, monsterTexture, spellTexture, deckTexture, graveTexture, extraTexture, fieldSpellTexture);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        field.draw(window, sf::Mouse::getPosition(window), centerBarTexture);
        window.display();
    }

    return 0;
}