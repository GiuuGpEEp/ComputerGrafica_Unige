#include "TurnManager.h"

TurnManager::TurnManager(int currentPlayer, GamePhase phase)
    : currentPlayer(currentPlayer), phase(phase) {}

void TurnManager::nextPhase() {
    switch(phase) {
        case GamePhase::Draw: phase = GamePhase::Standby; break;
        case GamePhase::Standby: phase = GamePhase::Main1; break;
        case GamePhase::Main1: phase = GamePhase::Battle; break;
        case GamePhase::Battle: phase = GamePhase::Main2; break;
        case GamePhase::Main2: phase = GamePhase::End; break;
        case GamePhase::End: endTurn(); break;
    }
}    