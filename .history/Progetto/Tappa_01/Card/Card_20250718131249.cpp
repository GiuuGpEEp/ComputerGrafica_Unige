//Definisco cosa fa la classe Card

#include "Card.h"

Card::Card(const std::string& name,const std::string& description, int atk, int def, sf::Vector2f pos, const sf::Font& font)
    : name(name), description(description), attack(atk), defense(def), label("", font) // initialize label with empty string and font
{
    shape.setSize(sf::Vector2f(100, 150));
    shape.setFillColor(sf::Color::Yellow);
    shape.setPosition(pos);

    label.setString(name + "\n" + description + "\nATK: " + std::to_string(attack) + "\tDEF: " + std::to_string(defense));
    label.setCharacterSize(14);
    label.setFillColor(sf::Color::Black);
    label.setPosition(pos + sf::Vector2f(5, 120)); //Prendo la posizione della carta e aggiungo un offset per il testo

}    