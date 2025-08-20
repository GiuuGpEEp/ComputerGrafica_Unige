#include "TurnManager.h"

TurnManager::TurnManager(int currentPlayerIndex, GamePhase startPhase)
    : currentPlayer(currentPlayerIndex), phase(startPhase) {}

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

void TurnManager::endTurn() {
    phase = GamePhase::Draw;
    currentPlayer = 1 - currentPlayer;
    ++turnCount;
}

int TurnManager::getTurnCount() const {
    return turnCount;
}

int TurnManager::getCurrentPlayerIndex() const {
    return currentPlayer;
}

int GamePhase::getPhase() const {
    return phase;
}