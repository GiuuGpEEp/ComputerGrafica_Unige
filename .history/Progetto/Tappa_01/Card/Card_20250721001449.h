#pragma once //Ho la garanzia che questo file non venga incluso più di una volta
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

class Card {
public:
    Card(const std::string& name, const std::string& description, int atk, int def, sf::Vector2f pos);
    void draw(sf::RenderWindow& window);
    bool isClicked(const sf::Vector2i& mousePos);
    void setPosition(sf::Vector2f pos);
    std::string getName() const;
    std::string getDescription() const;
    std::pair<int, int> getValues() const;

private:
    std::string name, description;
    int attack, defense;
    sf::RectangleShape shape;
    static sf::Font& getDefaultFont() {
        static sf::Font font;
        static bool loaded = false;
        if (!loaded) {
            // Carica un font di sistema o uno embedded, qui si può scegliere Arial come esempio
            font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");
            loaded = true;
        }
        return font;
    }
    sf::Text label{getDefaultFont(), "", 14};
};
