//Definisco cosa fa la classe Card

#include "Card.h"

Card::Card(const std::string& name,const std::string& description, int atk, int def, sf::Vector2f pos)
    : name(name), description(description), attack(atk), defense(def) //Notazione alternativa per l'inizializzazione dei membri = a this -> name = name
{
    // Carico il font - se fallisce uso il font di sistema
    if (!font.openFromFile("ITCKabelStdDemi.TTF")) {
        // Fallback a Calibri che sappiamo funziona
        if (!font.openFromFile("C:/Windows/Fonts/calibri.ttf")) {
            std::cerr << "ERRORE: Impossibile caricare nessun font!" << std::endl;
        }
    }
    
    //Inizializzo la forma della carta 
    shape.setSize(sf::Vector2f(100, 150));
    shape.setFillColor(sf::Color::Yellow);
    shape.setPosition(pos);
    
    //
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
    window.draw(shape);
    if (label.has_value()) {
        window.draw(label.value());
    }
}

//Controllo se la carta è stata cliccata - Verifico se il mouse è all'interno della forma della carta
bool Card::isClicked(const sf::Vector2i& mousePos) {
    return shape.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

//Imposto la posizione della carta
void Card::setPosition(sf::Vector2f pos) {
    shape.setPosition(pos);
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