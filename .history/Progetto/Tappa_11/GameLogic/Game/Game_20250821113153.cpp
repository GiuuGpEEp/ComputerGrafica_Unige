#include "Game.h"

Game::Game(const Player& p1, const Player& p2)
    : players{p1, p2} {
        for(auto& mz : monsterZones) mz.reserve(MONSTER_ZONE_SIZE + 2);
        // Nessun listener interno al TurnManager: tutta la propagazione passa da EventDispatcher
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

EventDispatcher& Game::events(){
    return dispatcher;
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
    dispatcher.emit(GameEventType::NormalSummon);
    return true;
}

void Game::resetNormalSummon(){
    normalSummonUsed = false;
}

const std::vector<Card>& Game::getMonsterZone() const {
    return monsterZones[turn.getCurrentPlayerIndex()];
}

std::vector<Card>& Game::getGraveyard(){
    return graveyard;
}

void Game::discardExcess(size_t handLimit){
    Player& p = current();
    auto &hand = p.getHand();
    while(hand.size() > handLimit){
        graveyard.push_back(hand.back());
        hand.pop_back();
    }
}

void Game::advancePhase(){
    GamePhase before = turn.getPhase();
    turn.nextPhase();
    if(turn.getPhase() != before){
        // Reset normal summon at entry of Draw or Standby
        if(turn.getPhase() == GamePhase::Draw || turn.getPhase() == GamePhase::Standby){
            resetNormalSummon();
        }
        dispatcher.emit(GameEventType::PhaseChange); // generic phase change
        if(turn.getPhase() == GamePhase::End){
            // limite mano verrà applicato su endTurn oppure qui se serve
        }
    }
}

void Game::endTurn(){
    // Prima applica limite mano
    discardExcess(7);
    turn.endTurn();
    // New turn always starts at Draw, reset summon availability
    resetNormalSummon();
    dispatcher.emit(GameEventType::PhaseChange); // nuova Draw phase
}

