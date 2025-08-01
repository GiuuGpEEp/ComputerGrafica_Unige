#pragma once //Ho la garanzia che questo file non venga incluso più di una volta
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include "../../resources/GameState.h"
#include <optional>

class Card {
public:
    Card(const std::string& name, const std::string& description, int atk, int def, sf::Vector2f pos, sf::Vector2f size, sf::Texture& textureRef);
    ~Card(); // Distruttore esplicito della classe 
    
    void draw(sf::RenderWindow& window);
    bool isClicked(const sf::Vector2i& mousePos);
    void setPosition(sf::Vector2f pos);
    void setTexture(sf::Texture& texture);
    void setTextureRect(const sf::IntRect& rect);
    void setSize(sf::Vector2f size);
    std::string getName() const;
    std::string getDescription() const;
    std::pair<int, int> getValues() const;

private:
    std::string name, description;
    int attack, defense;
    sf::Sprite sprite;
};
