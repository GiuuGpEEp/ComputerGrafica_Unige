//Definisco cosa fa la classe Card

#include "Card.h"

//Costruttore
Card::Card(const std::string& name,const std::string& description, int atk, int def, sf::Vector2f pos)
    : name(name), description(description), attack(atk), defense(def) //Notazione alternativa per l'inizializzazione dei membri = a this -> name = name
{
    //Carico il font
    if (!font.loadFromFile("../../resources/ITCKabelStdDemi.TTF")) {
        std::cerr << "Errore: Impossibile caricare il font!" << std::endl;
        // Usa un font di default del sistema se disponibile
    }
    
    //Inizializzo la forma della carta 
    shape.setSize(sf::Vector2f(100, 150));
    shape.setFillColor(sf::Color::Yellow);
    shape.setPosition(pos);
    
    //Inizializzo il testo della carta
    label.setFont(font);
    label.setString(name + "\n" + description + "\nATK: " + std::to_string(attack) + "\tDEF: " + std::to_string(defense));
    label.setFillColor(sf::Color::Black);
    label.setCharacterSize(14);
    label.setPosition(pos + sf::Vector2f(5, 120)); //Prendo la posizione della carta e aggiungo un offset per il test  
}    

//Disegno la carta
void Card::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(label);
}

//Controllo se la carta è stata cliccata - Verifico se il mouse è all'interno della forma della carta
bool Card::isClicked(const sf::Vector2i& mousePos) {
    return shape.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

//Imposto la posizione della carta
void Card::setPosition(sf::Vector2f pos) {
    shape.setPosition(pos);
    label.setPosition(pos + sf::Vector2f(5, 120)); 
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