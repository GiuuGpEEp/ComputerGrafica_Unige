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
    if(phase != GamePhase::Draw) // Già notifica
        notifyPhaseListeners();
}

void TurnManager::endTurn() {
    phase = GamePhase::Draw;
    currentPlayer = 1 - currentPlayer;
    ++turnCount;
    notifyPhaseListeners();
}

int TurnManager::getTurnCount() const {
    return turnCount;
}

int TurnManager::getCurrentPlayerIndex() const {
    return currentPlayer;
}

GamePhase TurnManager::getPhase() const {
    return phase;
}

void TurnManager::addPhaseListener(const std::function<void(GamePhase)>& cb){
    phaseListeners.push_back(cb);
}

void TurnManager::notifyPhaseListeners(){
    for(auto& cb : phaseListeners) cb(phase);
}