#include "Game.h"

Game::Game(const Player& p1, const Player& p2)
    : players{p1, p2} {}
    
void Game::start() {
    if (started) return;
    // pesca iniziale 5 carte ciascuno
    for (int i = 0; i < 5; ++i) {
        players[0].drawOne();
        players[1].drawOne();
    }
    started = true;
} 



Player& Game::current() {
    return players[turn.currentPlayer];
}

Player& Game::opponent() {
    return players[1 - turn.currentPlayer];
}