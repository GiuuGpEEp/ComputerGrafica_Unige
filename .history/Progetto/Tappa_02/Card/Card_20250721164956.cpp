//Definisco cosa fa la classe Card

#include "Card.h"

Card::Card(const std::string& name,const std::string& description, int atk, int def, sf::Vector2f pos, bool flipped)
    : name(name), description(description), attack(atk), defense(def) //Notazione alternativa per l'inizializzazione dei membri = a this -> name = name
{
    // Carico il font - se fallisce uso il font di sistema
    auto fontResult = sf::Font::fromFile("../../Progetto/resources/ITCKabelStdDemi.TTF");
    if (!fontResult) {
        fontResult = sf::Font::fromFile("C:/Windows/Fonts/calibri.ttf");
        if (!fontResult) {
            std::cerr << "ERRORE: Impossibile caricare nessun font!" << std::endl;
        }
    }
    if (fontResult) {
        font = std::move(*fontResult);
    }

    auto flippedTextureResult = sf::Texture::fromFile("../../Progetto/resources/Texture1.png");
    if (!flippedTextureResult) {
        std::cerr << "ERRORE: Impossibile caricare la texture della carta coperta!" << std::endl;
    } else {
        textureFlipped = std::move(*flippedTextureResult);
    }

    auto notFlippedTextureResult = sf::Texture::fromFile("../../Progetto/resources/CardNotSet.png");
    if (!notFlippedTextureResult) {
        std::cerr << "ERRORE: Impossibile caricare la texture della carta scoperta!" << std::endl;
    } else {
        textureNotFlipped = std::move(*notFlippedTextureResult);
    }

    //Inizializzo lo sprite della carta
    if(flipped) {
        sprite.setTexture(&textureFlipped);
    } else {
        sprite.setTexture(&textureNotFlipped);
    }
    sprite.setScale({10.f, 10.f});
    sprite.setPosition(pos);
  
    //Emplace permette di costruire l'oggetto direttamente in-place nel container, evitando una copia
    label.emplace(font, name + "\n" + description + "\nATK: " + std::to_string(attack) + "\tDEF: " + std::to_string(defense), 12);
    label->setFillColor(sf::Color::Black);
    label->setPosition(pos + sf::Vector2f(5, 10)); //Prendo la posizione della carta e aggiungo un offset per il test  
}    

//Distruttore
Card::~Card() {
    // Distruttore pulito - debug rimosso
}

//Disegno la carta
void Card::draw(sf::RenderWindow& window) {
    window.draw(sprite);
    //In questa prima tappa il testo lo visualizzo all'interno della carta stessa
    if (label.has_value()) {
        window.draw(label.value());
    }
}

//Controllo se la carta è stata cliccata - Verifico se il mouse è all'interno della forma della carta
bool Card::isClicked(const sf::Vector2i& mousePos) {
    return sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

//Imposto la posizione della carta
void Card::setPosition(sf::Vector2f pos) {
    sprite.setPosition(pos);
    if (label.has_value()) {
        label->setPosition(pos + sf::Vector2f(5, 10)); 
    }
}

//Restituisco il nome della carta
std::string Card::getName() const {
    return name;
}

//Restituisco la descrizione della carta
std::string Card::getDescription() const {
    return description;
}

//Restituisco i valori di attacco e difesa della carta (si usa const per garantire che la funzione non modifichi lo stato dell'oggetto)
std::pair<int, int> Card::getValues() const {
    return {attack, defense};
}