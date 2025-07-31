#pragma once //Ho la garanzia che questo file non venga incluso più di una volta
#include <SFML/Graphics.hpp>
#include "../Card/Card.h"
#include <vector>

class Deck{
    public:
        Deck(Vector2f pos, sf::Vector2f size, sf::Texture& textureRef);
        void shuffle();
        Card drawCard(int numberCards);
    private:
        std::vector<Card> cards;

};