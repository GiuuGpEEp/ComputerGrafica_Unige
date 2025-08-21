#include "Game.h"
#include "../DrawController/DrawController.h"

Game::Game(const Player& p1, const Player& p2)
    : players{p1, p2} {
        for(auto& mz : monsterZones) mz.reserve(MONSTER_ZONE_SIZE);
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
    // Distribuzione mano iniziale: 5 carte al primo giocatore, 5 al secondo (senza animazione: queue nel draw controller)
    if(drawCtrl){
        drawCtrl->queueDraw(5); // starting player hand
        // opzionale: dare anche al secondo giocatore già 5 carte: per ora NO, verranno pescate all'inizio del suo primo turno se necessario.
    }
    startTurn(); 

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
    if(handIndex >= hand.size()) return false;
    if(!canNormalSummon()) return false;
    // Usa API generica di movimento (emette anche CardMoved)
    if(!moveCard(CardZone::Hand, CardZone::MonsterZone, handIndex)) return false;
    normalSummonUsed = true;
    dispatcher.emit(GameEventType::NormalSummon); // evento specifico summon
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
        // sposta ultima carta (hand.size()-1) nel cimitero tramite API (emette eventi)
        moveCard(CardZone::Hand, CardZone::Graveyard, hand.size()-1);
    }
}

std::vector<Card> Game::extractExcessCards(size_t handLimit){
    std::vector<Card> extracted;
    Player& p = current();
    auto &hand = p.getHand();
    while(hand.size() > handLimit){
        extracted.push_back(hand.back());
        hand.pop_back();
    }
    return extracted; // il chiamante deciderà quando inserirle nel graveyard (post animazione)
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
        // Auto draw quando si entra in Draw Phase
        if(turn.getPhase() == GamePhase::Draw && drawCtrl){
            startTurn();
        }
        if(turn.getPhase() == GamePhase::Standby){
            processEnterStandby();
        }
        if(turn.getPhase() == GamePhase::End){
            processEnterEndPhase();
        }
    }
}

void Game::endTurn(){
    dispatcher.emit(GameEventType::TurnEnd);
    turn.endTurn(); // ora siamo già nella Draw del prossimo giocatore
    firstTurn = false; // dopo la conclusione del primissimo turno
    startTurn();
}

void Game::attachDrawController(DrawController* controller){
    drawCtrl = controller;
}

std::vector<Card> Game::handleEndPhase(size_t handLimit){
    return discardExcess(handLimit);
}

void Game::setHandLimit(size_t limit){ handLimit = limit; }
void Game::setDiscardCallback(std::function<void(std::vector<Card>&&)> cb){ discardCallback = std::move(cb); }

void Game::fastForwardToEndPhase(){
    while(turn.getPhase() != GamePhase::End){
        advancePhase();
    }
}

bool Game::shouldAutoEndTurn() const { return pendingAutoTurnEnd; }

void Game::onDiscardAnimationFinished(){
    // Chiamato dal layer esterno quando animazione scarto termina -> fine turno immediata
    pendingAutoTurnEnd = false;
    endTurn();
}

void Game::processEnterEndPhase(){
    // Se mano eccede limite, estrai e delega animazione
    Player& p = current();
    auto &hand = p.getHand();
    if(hand.size() > handLimit){
        if(discardCallback){
            discardCallback(handleEndPhase(handLimit));
        } else {
            // fallback immediato: scarta direttamente nel graveyard senza animazione
            discardExcess(handLimit);
        }
    } else {
        // nessun eccesso -> programma auto end turno (consumato dal main o immediatamente se si vuole)
        pendingAutoTurnEnd = true;
    }
}

void Game::processEnterStandby(){
    // Auto-skip Standby se nessun effetto (placeholder: sempre skip per ora)
    bool shouldSkip = true; // TODO: interrogare effect system
    if(shouldSkip){
        dispatcher.emit(GameEventType::StandbySkip);
        // Avanza direttamente a Main1 (internal state change)
        turn.nextPhase();
        dispatcher.emit(GameEventType::PhaseChange);
    }
}

void Game::startTurn(){
    resetNormalSummon();
    dispatcher.emit(GameEventType::TurnStart);
    // Siamo nella Draw Phase appena impostata da TurnManager::endTurn() oppure dall'inizio partita
    if(drawCtrl && !firstTurn){
        drawCtrl->queueDraw(1);
    }
}

bool Game::moveCard(CardZone from, CardZone to, size_t indexFrom){
    Player &p = current();
    std::vector<Card>* source = nullptr;
    std::vector<Card>* dest = nullptr;
    switch(from){
        case CardZone::Hand: source = &p.getHand(); break;
        case CardZone::MonsterZone: source = &monsterZones[turn.getCurrentPlayerIndex()]; break;
        case CardZone::Graveyard: source = &graveyard; break;
        case CardZone::Deck: /* non supportato estrarre per indice diretto */ return false;
    }
    if(!source) return false;
    if(indexFrom >= source->size()) return false;
    Card card = (*source)[indexFrom];
    source->erase(source->begin()+indexFrom);
    // destinazione
    switch(to){
        case CardZone::Hand: dest = &p.getHand(); break;
        case CardZone::MonsterZone:
            dest = &monsterZones[turn.getCurrentPlayerIndex()];
            if(dest->size() >= MONSTER_ZONE_SIZE){ return false; }
            break;
        case CardZone::Graveyard: dest = &graveyard; break;
        case CardZone::Deck: return false; // no reinsert deck for now
    }
    if(!dest) return false;
    dest->push_back(card);
    dispatcher.emit(GameEventType::CardMoved);
    if(to == CardZone::Graveyard){
        dispatcher.emit(GameEventType::CardSentToGrave);
    }
    return true;
}
