#pragma once //Ho la garanzia che questo file non venga incluso più di una volta
#include <SFML/Graphics.hpp>
#include "../Card/Card.h"
#include <vector>

class Deck{
    public:
        Deck(sf::Vector2f pos, sf::Vector2f deckCardSize, sf::Texture& textureRef, numberDeckCard);
        void shuffle();
        Card drawCard(int drawCards);
    private:
        std::vector<Card> cards;
        int cardsSize;

};