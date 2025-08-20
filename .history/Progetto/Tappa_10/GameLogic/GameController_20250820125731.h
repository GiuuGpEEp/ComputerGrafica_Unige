#pragma once
#include <vector>
#include <optional>
#include <SFML/Graphics.hpp>
#include "../Deck/Deck.h"
#include "../Field/Field.h"
#include "../DrawAnimation/DrawAnimation.h"
#include "Phase.h"

class GameController {
public:
    GameController(Field& fieldRef,
                   std::vector<Card>& handRef,
                   std::vector<Card>& fieldCardsRef,
                   Deck& sharedDeck,
                   std::vector<DrawAnimation>& drawAnims,
                   int& cardsToDrawRef, sf::Vector2f slotSizeRef);

    void onEnterPlaying();                 // chiamato quando passi a GameState::Playing
    void handleEvent(const sf::Event& ev); // input per avanzare fasi / azioni
    void update(float dt);                 // logica per fase corrente
    void drawUI(sf::RenderWindow& win, sf::Font& font); // pannellino fasi / turno

    

private:
    Field& field;
    std::vector<Card>& hand;
    std::vector<Card>& fieldCards;
    Deck& deck;
    std::vector<DrawAnimation>& animations;
    int& cardsToDraw;
    sf::Vector2f slotSize;

    PlayerState players[2];
    int activePlayer = 0;
    Phase phase = Phase::Draw;
    bool drawDoneThisTurn = false;
    bool normalSummonUsed = false;

    // Helpers
    void startTurn();
    void nextPhase();
    void endTurn();

    void doDrawPhase();
    bool tryNormalSummon(size_t handIndex, const sf::Vector2f& mousePos);
    void doBattlePhaseAttack(); // placeholder (poi selezione bersaglio)
};