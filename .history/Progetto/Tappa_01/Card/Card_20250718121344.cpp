//Definisco cosa fa la classe Card

#include "Card.h"

Card::Card(const std::string& name, int atk, int def, sf::Vector2f pos){
    this->name = name;
    this->attack = atk;
    this->defense = def;
    this->shape.setPosition(pos);
}