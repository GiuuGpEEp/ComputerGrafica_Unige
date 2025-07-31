#pragma once //Ho la garanzia che questo file non venga incluso più di una volta
#include <SFML/Graphics.hpp>
#include "../Card/Card.h"
#include <vector>

class Deck{
    public:
        Deck(sf::Vector2f pos, sf::Vector2f deckCardSize, sf::Vector2f slotSize, sf::Texture& textureRef, int numberDeckCards);
        void shuffle();
        Card drawCard(int drawCards);
        
        void draw(sf::RenderWindow& window);
        

    private:
        std::vector<Card> cards;
        int cardsSize;

};