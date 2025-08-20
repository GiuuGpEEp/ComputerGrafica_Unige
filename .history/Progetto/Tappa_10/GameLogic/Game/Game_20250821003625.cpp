#include "Game.h"

Game::Game(const Player& p1, const Player& p2)
    : players{p1, p2} {
        for(auto& mz : monsterZones) mz.reserve(MONSTER_ZONE_SIZE);
        // Listener fasi: reset normal summon nelle fasi Draw e Standby
        turn.addPhaseListener([this](GamePhase ph){
            if(ph == GamePhase::Draw || ph == GamePhase::Standby ){
                resetNormalSummon();
            }
        });
    }
    
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

bool Game::canNormalSummon() const {
    if(normalSummonUsed) return false;
    if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
    return monsterZones[turn.getCurrentPlayerIndex()].size() < MONSTER_ZONE_SIZE;
}

bool Game::tryNormalSummon(size_t handIndex){
    Player& p = current();
    auto& hand = p.getHand();
    int idx = turn.getCurrentPlayerIndex();
    if(handIndex >= hand.size()) return false;
    if(!canNormalSummon()) return false;
    monsterZones[idx].push_back(hand[handIndex]);
    hand.erase(hand.begin() + handIndex);
    normalSummonUsed = true;
    return true;
}

void Game::resetNormalSummon(){
    normalSummonUsed = false;
}

const std::vector<Card>& Game::getMonsterZone() const {
    return monsterZones[turn.getCurrentPlayerIndex()];
}

