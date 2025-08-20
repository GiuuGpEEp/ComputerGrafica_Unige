#pragma once
#include "../GamePhases.h"

class TurnManager {
public:
    TurnManager(int currentPlayer, GamePhase phase);

    void nextPhase();
    void endTurn();

    int getTurnCount() const;
    int getCurrentPlayerIndex() const;
    GamePhase getPhase() const;

private:
    int turnCount = 1; // parte dal turno 1 del Player 1
    int currentPlayer = 0; // 0 = Player 1, 1 = Player 2
    GamePhase phase = GamePhase::Draw;    
};
