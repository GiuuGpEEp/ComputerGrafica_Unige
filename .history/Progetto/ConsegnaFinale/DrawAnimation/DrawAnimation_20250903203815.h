#pragma once //Ho la garanzia che questo file non venga incluso più di una volta
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include "../Card/Card.h"
#include "../../resources/data/GameState.h"
#include "../Utils/auxFunc.h"
#include "../TextureManager/TextureManager.h"
#include <optional>


enum class DrawAnimationPhases{
    MovingOut,
    MovingToPause,
    ShowCard,
    MovingHand,
    Done
};

class DrawAnimation{
    public:
    DrawAnimation(Card drawnCard, DrawAnimationPhases phase, sf::Vector2f startPos, sf::Vector2f pausePos, TextureManager& textureManager, bool skipCenterReveal = false);
        bool moveTowards(sf::Vector2f& current, const sf::Vector2f& target, float speed, float deltaTime, Card& card);
        DrawAnimationPhases update(
            float moveSpeed,
            float deltaTime,
            sf::Texture& texture,
            std::vector<Card>& cards,
            sf::Vector2u& windowSize,
            sf::Vector2f& cardSize,
            float spacing,
            float y,
            int HAND_MAXSIZE,
            bool skipPause,           // true quando il giocatore clicca per saltare l'attesa al centro
            bool skipCenterReveal,    // true per saltare completamente la rivelazione al centro (pescata dell'avversario)
            TextureManager& textureManager
        );
        float pauseDuration = 0.f;
        bool pauseDurationSet = false;
        void setHandPos(std::vector<Card>& cards, sf::Vector2u& windowSize, sf::Vector2f& cardSize, float spacing, float y, int HAND_MAXSIZE); //Da usare nell'update
        void draw(sf::RenderWindow& window);
        bool isFinished() const;
        Card& getCard() const;

    private:
        Card card;
        DrawAnimationPhases phase = DrawAnimationPhases::MovingOut;
        sf::Vector2f startPos;
        sf::Vector2f pausePos; //Posizione di pausa, per mostrare la carta prima di spostarla in mano
        sf::Vector2f handPos; //Posizione finale in mano, calcolata con updateHandPositions
        float pauseTime = 0.f;
        bool atPause = false; 
        bool finished = false; 
        bool constructedSkipCenterReveal = false;
    public:
        bool isSkipCenterReveal() const { return constructedSkipCenterReveal; }
};