#pragma once
#include "GamePhases.h"

class TurnManager {
public:

    TurnManager(int currentPlayer, GamePhase phase);

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
