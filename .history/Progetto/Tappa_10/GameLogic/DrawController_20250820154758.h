#pragma once
#include <vector>
#include "../DrawAnimation/DrawAnimation.h"
#include "../Deck/Deck.h"
#include "../TextureManager/TextureManager.h"
#include "../auxFunc.h"

class DrawController {
public:
    void queueDraw(int n){ cardsToDraw += n; }
    int pending() const { return cardsToDraw; }

    // Avvia animazione se idle e ci sono pescate da fare
    void update(float deltaTime,
                bool mousePressed,
                Deck& deck,
                std::vector<Card>& hand,
                const sf::Vector2u& windowSize,
                const sf::Vector2f& cardSize,
                float spacing,
                float handY,
                int handMaxSize,
                const sf::Vector2f& deckSlotPos,
                sf::Texture& textureNonFlipped,
                sf::Texture& textureFlipped,
                TextureManager& textureManager){
        if(cardsToDraw > 0 && animations.empty() && !deck.isEmpty()){
            Card next = deck.drawCard();
            --cardsToDraw;
            DrawAnimation anim(next, DrawAnimationPhases::MovingOut, deckSlotPos,
                sf::Vector2f(windowSize.x/2.f - cardSize.x/2.f, windowSize.y/2.f - cardSize.y/2.f), textureManager);
            animations.push_back(anim);
        }
        if(!animations.empty()){
            DrawAnimationPhases ph = animations.front().update(MOVEMENT_SPEED_INTERNAL, deltaTime, textureNonFlipped, hand, windowSize, cardSize, spacing, handY, handMaxSize, mousePressed, textureManager);
            if(ph == DrawAnimationPhases::ShowCard){
                showDrawnCardDetails = true;
                tmpCard = animations.front().getCard();
            }
            if(ph == DrawAnimationPhases::Done){
                showDrawnCardDetails = false;
                // Aggiunge al vettore mano (che rappresenta Player.hand)
                hand.push_back(animations.front().getCard());
                animations.erase(animations.begin());
                updateHandPositions(hand, windowSize, cardSize, spacing, handY, handMaxSize);
                deck.resetDeckCardPositions(deckSlotPos, cardSize, cardSize, textureFlipped); // cardSize for slotSize placeholder
            }
        }
    }

    bool isShowingDetails() const { return showDrawnCardDetails; }
    const Card& detailCard() const { return tmpCard; }
    bool idle() const { return animations.empty(); }
    void reset(){ animations.clear(); cardsToDraw = 0; showDrawnCardDetails = false; }

private:
    std::vector<DrawAnimation> animations;
    int cardsToDraw = 0;
    bool showDrawnCardDetails = false;
    Card tmpCard{"","",0,0,{0,0},{0,0},*(sf::Texture*)nullptr,Type::Monster,Attribute::None,0,{}}; // placeholder safe replaced before access
    const float MOVEMENT_SPEED_INTERNAL = 2000.f;
};
