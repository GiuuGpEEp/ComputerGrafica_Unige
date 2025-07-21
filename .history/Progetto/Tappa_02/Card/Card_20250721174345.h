#pragma once //Ho la garanzia che questo file non venga incluso più di una volta
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <optional>

class Card {
public:
    Card(const std::string& name, const std::string& description, int atk, int def, sf::Vector2f pos, sf::Texture& textureRef);
    ~Card(); // Distruttore esplicito della classe 
    void draw(sf::RenderWindow& window);
    bool isClicked(const sf::Vector2i& mousePos);
    void setPosition(sf::Vector2f pos);
    std::string getName() const;
    std::string getDescription() const;
    std::pair<int, int> getValues() const;

private:
    std::string name, description;
    int attack, defense;
    sf::Sprite sprite;

};
