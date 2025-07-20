#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Card {
public:
    Card(const std::string& name, int atk, int def, sf::Vector2f pos);
    void draw(sf::RenderWindow& window);
    bool isClicked(const sf::Vector2i& mousePos);
    void setPosition(sf::Vector2f pos);
    std::string getName() const;

private:
    std::string name;
    int attack, defense;
    sf::RectangleShape shape;
    sf::Font font;
    sf::Text label;
};
