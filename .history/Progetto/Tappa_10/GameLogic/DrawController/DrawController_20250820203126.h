#pragma once
#include <vector>
#include <optional>
#include "../../DrawAnimation/DrawAnimation.h"
#include "../../Deck/Deck.h"
#include "../../TextureManager/TextureManager.h"
#include "../../auxFunc.h"

class DrawController {
    public:

        DrawController();

        void queueDraw(int n);
        int pending() const;

        // Avvia animazione se idle e ci sono pescate da fare
        void update(float deltaTime,
                    bool mousePressed,
                    Deck& deck,
                    std::vector<Card>& hand,
                    sf::Vector2u windowSize,
                    sf::Vector2f cardSize,
                    float spacing,
                    float handY,
                    int handMaxSize,
                    sf::Vector2f deckSlotPos,
                    sf::Texture& textureNonFlipped,
                    sf::Texture& textureFlipped,
                    TextureManager& textureManager
        );            

        bool isShowingDetails() const; //Riferimento alla fase in cui la carte si ferma durante le animazioni
        const Card& detailCard() const;
        bool idle() const;
        void reset();

        // Disegno delle animazioni ancora in corso
        void draw(sf::RenderWindow& window){
            for(auto& a : animations) a.draw(window);
        }

    private:
        std::vector<DrawAnimation> animations;
        int cardsToDraw = 0;
        bool showDrawnCardDetails = false;
        std::optional<Card> detailCardOpt; 
        const float MOVEMENT_SPEED_INTERNAL = 2000.f;
};
