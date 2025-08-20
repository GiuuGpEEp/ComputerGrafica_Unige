#pragma once
#include <array>
#include <SFML/Graphics.hpp>
#include "Phase.h"
#include "PlayerState.h"
#include "../Field/Field.h"
#include "../DrawAnimation/DrawAnimation.h"
#include "../Deck/Deck.h"

class GameController {
public:
    GameController(Field& fieldRef,
                   Deck& deckP1,
                   Deck& deckP2,
                   std::array<std::vector<Card>,2>& handsRef,
                   std::array<std::vector<Card>,2>& fieldRefs,
                   std::vector<DrawAnimation>& drawAnims,
                   int& cardsToDrawRef);

    void onEnterPlaying();
    void handleEvent(const sf::Event& ev);
    void update(float dt);
    void drawUI(sf::RenderWindow& win, sf::Font& font);

    int getActivePlayer() const { return activePlayer; }
    Phase getPhase() const { return phase; }

private:
    Field& field;
    Deck& deck1;
    Deck& deck2;
    std::array<std::vector<Card>,2>& hands;
    std::array<std::vector<Card>,2>& fields;
    std::vector<DrawAnimation>& animations;
    int& cardsToDraw; // usato solo per P1 (animata)

    std::array<PlayerState,2> players;
    int activePlayer = 0;
    Phase phase = Phase::Draw;
    bool drawDoneThisTurn = false;
    bool normalSummonUsed = false;

    void startTurn();
    void nextPhase();
    void endTurn();
    void doDrawPhase();
    bool tryNormalSummon(size_t handIndex, sf::);
};