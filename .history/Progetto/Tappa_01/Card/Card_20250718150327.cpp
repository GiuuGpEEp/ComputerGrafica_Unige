//Definisco cosa fa la classe Card

#include "Card.h"

// Implementazione del costruttore della classe Card
Card::Card(const std::string& name,const std::string& description, int atk, int def, sf::Vector2f pos)
    : name(name), description(description), attack(atk), defense(def) //Notazione alternativa per l'inizializzazione dei membri = a this -> name = name
{
    //Inizializzo la forma della carta 
    shape.setSize(sf::Vector2f(100, 150));
    shape.setFillColor(sf::Color::Yellow);
    shape.setPosition(pos);
    
    //Inizializzo il testo della carta
    label.setString(name + "\n" + description + "\nATK: " + std::to_string(attack) + "\tDEF: " + std::to_string(defense));
    label.setFillColor(sf::Color::Black);
    label.setPosition(pos + sf::Vector2f(5, 120)); //Prendo la posizione della carta e aggiungo un offset per il test  
}    