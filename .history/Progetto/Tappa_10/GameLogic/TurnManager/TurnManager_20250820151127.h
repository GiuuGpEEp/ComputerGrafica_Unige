#pragma once
#include "GamePhases.h"

class TurnManager {
public:
    int turnCount = 1; // parte dal turno 1 del Player 1
    int currentPlayer = 0; // 0 = Player 1, 1 = Player 2
    GamePhase phase = GamePhase::Draw;

    TurnManager(int currentPlayer, );

    void nextPhase(){
        switch(phase){
            case GamePhase::Draw: phase = GamePhase::Standby; break;
            case GamePhase::Standby: phase = GamePhase::Main1; break;
            case GamePhase::Main1: phase = GamePhase::Battle; break;
            case GamePhase::Battle: phase = GamePhase::Main2; break;
            case GamePhase::Main2: phase = GamePhase::End; break;
            case GamePhase::End: endTurn(); break;
        }
    }

    void endTurn(){
        phase = GamePhase::Draw;
        currentPlayer = 1 - currentPlayer;
        ++turnCount;
    }
};
