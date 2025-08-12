#pragma once //Ho la garanzia che questo file non venga incluso più di una volta
#include <SFML/Graphics.hpp>
#include "../Card/Card.h"
#include "../../resources/data/GameState.h"
#include <vector>

class Deck{
    public:
        Deck(sf::Vector2f pos, sf::Vector2f deckCardSize, sf::Vector2f slotSize, sf::Texture& textureRef, int numberDeckCards);
        void shuffle();
        Card drawCard();
        int getSize() const;
        bool isEmpty() const;
        void draw(sf::RenderWindow& window, const sf::Vector2i& mousePos, const sf::Font& font, sf::Vector2f slotPos, sf::Vector2f slotSize, GameState gamestate);
        bool isAnimationFinished() const;
        void setAnimationFinished();
        void animate(float deltaTime);

    private:
        float deckAlpha = 0.f;
        std::vector<Card> cards;
        int cardsSize;
        bool animationFinished;

};