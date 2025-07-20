//Definisco cosa fa la classe Card

#include "Card.h"

Card::Card(const std::string& name, int atk, int def, sf::Vector2f pos)
    : name(name), attack(atk), defense(def) //Notazione alternativa per l'inizializzazione dei membri = a 
{
    
}    