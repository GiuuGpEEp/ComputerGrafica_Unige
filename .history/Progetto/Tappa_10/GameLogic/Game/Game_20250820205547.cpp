#include "Game.h"

Game::Game(const Player& p1, const Player& p2)
    : players{p1, p2} {}
    
void Game::start() {
    if (started) return;
    // Reset stato partita senza effettuare pescate (delegate a DrawController)
    // Svuota le mani dei giocatori per sicurezza
    for(auto& p : players){
        p.clearHand();
    }
    started = true;
} 

bool Game::isStarted() const {
    return started;
}

Player& Game::current() {
    return players[turn.getCurrentPlayerIndex()];
}

Player& Game::opponent() {
    return players[1 - turn.getCurrentPlayerIndex()];
}

TurnManager& Game::getTurn(){
    return turn;
}

