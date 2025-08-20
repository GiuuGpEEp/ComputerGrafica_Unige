#include "TurnManager.h"

TurnManager::TurnManager(Player& p1, Player& p2)
    : player1(p1), player2(p2), currentPlayer(&player1) {}

void TurnManager::endTurn() {
    currentPlayer = (currentPlayer == &player1) ? &player2 : &player1;
}

Player& TurnManager::getCurrentPlayer() {
    return *currentPlayer;
}
