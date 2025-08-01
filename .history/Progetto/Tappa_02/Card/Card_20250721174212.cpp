//Definisco cosa fa la classe Card

#include "Card.h"

Card::Card(const std::string& name, const std::string& description, int atk, int def, sf::Vector2f pos, sf::Texture& textureRef)
    : name(name), description(description), attack(atk), defense(def), sprite(textureRef) //Inizializzo sprite con textureRef
{
    //// Carico il font - se fallisce uso il font di sistema
    //if (!font.openFromFile("../../Progetto/resources/ITCKabelStdDemi.TTF")) {
    //    // Fallback a Calibri che sappiamo funziona
    //    if (!font.openFromFile("C:/Windows/Fonts/calibri.ttf")) {
    //        std::cerr << "ERRORE: Impossibile caricare nessun font!" << std::endl;
    //    }
    //}

    //Inizializzo lo sprite della carta
    sprite.setTexture(textureRef);

    // Imposto le dimensioni desiderate per lo sprite della carta in modo dinamico
    sf::Vector2f desiredSize(128.f, 180.f);
    sf::FloatRect bounds = sprite.getLocalBounds();
    sf::Vector2f scale(desiredSize.x / bounds.size.x, desiredSize.y / bounds.size.y);
    sprite.setScale(scale);
    
    sprite.setPosition(pos);
  
    ////Emplace permette di costruire l'oggetto direttamente in-place nel container, evitando una copia
    //label.emplace(font, name + "\n" + description + "\nATK: " + std::to_string(attack) + "\tDEF: " + std::to_string(defense), 12);
    //label->setFillColor(sf::Color::Black);
    //label->setPosition(pos + sf::Vector2f(5, 10)); //Prendo la posizione della carta e aggiungo un offset per il test  
}    

//Distruttore
Card::~Card() {
    // Distruttore pulito - debug rimosso
}

//Disegno la carta
void Card::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

//Controllo se la carta è stata cliccata - Verifico se il mouse è all'interno della forma della carta
bool Card::isClicked(const sf::Vector2i& mousePos) {
    return sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

//Imposto la posizione della carta
void Card::setPosition(sf::Vector2f pos) {
    sprite.setPosition(pos);
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