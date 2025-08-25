#include "Game.h"
#include "../DrawController/DrawController.h"
#include "Effects/ICardEffect.h"
#include "Effects/EffectSystem.h"

Game::Game(const Player& p1, const Player& p2)
    : players{p1, p2} {
        for(auto& mz : monsterZones) mz.reserve(MONSTER_ZONE_SIZE);
    // Nessun listener interno al TurnManager: tutta la propagazione passa da EventDispatcher
    monsterHasAttacked[0].clear();
    monsterHasAttacked[1].clear();
        banished[0].clear();
        banished[1].clear();

    // Forward automatico verso il sistema effetti per alcuni eventi non ancora inoltrati direttamente
    // Nota: evitiamo di duplicare eventi già inoltrati esplicitamente (TurnStart, PhaseChange, BattleStart/End, AttackDeclared/Resolved)
    dispatcher.subscribe(GameEventType::NormalSummon, [this]() { this->dispatchEffects(GameEventType::NormalSummon); });
    dispatcher.subscribe(GameEventType::MonstersTributed, [this]() { this->dispatchEffects(GameEventType::MonstersTributed); });
    dispatcher.subscribe(GameEventType::CardSentToGrave, [this]() { this->dispatchEffects(GameEventType::CardSentToGrave); });
    dispatcher.subscribe(GameEventType::LifePointsChanged, [this]() { this->dispatchEffects(GameEventType::LifePointsChanged); });
    dispatcher.subscribe(GameEventType::DirectAttack, [this]() { this->dispatchEffects(GameEventType::DirectAttack); });
    dispatcher.subscribe(GameEventType::TurnEnd, [this]() { this->dispatchEffects(GameEventType::TurnEnd); });
    dispatcher.subscribe(GameEventType::Win, [this]() { this->dispatchEffects(GameEventType::Win); });
    dispatcher.subscribe(GameEventType::Lose, [this]() { this->dispatchEffects(GameEventType::Lose); });
    // DrawStart/DrawEnd sono emessi dal DrawController sul dispatcher del Game
    dispatcher.subscribe(GameEventType::DrawStart, [this]() { this->dispatchEffects(GameEventType::DrawStart); });
    dispatcher.subscribe(GameEventType::DrawEnd, [this]() { this->dispatchEffects(GameEventType::DrawEnd); });
    }
    
void Game::start() {
    if (started) return;
    // Reset dello stato partita senza effettuare pescate (delegato al DrawController)
    // Svuota le mani dei giocatori per sicurezza
    for(auto& p : players){
        p.clearHand();
    }
    started = true;
    // Distribuzione mano iniziale: 5 carte al primo giocatore, 5 al secondo (senza animazione: messa in coda nel DrawController)
    if(drawCtrl){
        drawCtrl->queueDraw(5); // starting player hand
    // Opzionale: dare anche al secondo giocatore già 5 carte. Per ora NO: verranno pescate all'inizio del suo primo turno se necessario.
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
    // Controlli di fase e uso della Normal Summon
    if(normalSummonUsed) return false;
    if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
    // Calcola i tributi richiesti PRIMA del controllo della capacità della zona mostri
    int tributesNeeded = 0;
    if(auto lvl = hand[handIndex].getLevelOrRank(); lvl.has_value()){
        tributesNeeded = requiredTributesFor(hand[handIndex]);
    }
    if(tributesNeeded > 0){
    // Anche con zona piena i tributi liberano spazio: consenti l'avvio se ci sono abbastanza mostri da tributare
        return beginNormalSummonWithTributes(handIndex);
    }
    // Nessun tributo richiesto: serve spazio libero nella zona mostri
    if(monsterZones[turn.getCurrentPlayerIndex()].size() >= MONSTER_ZONE_SIZE) return false;
    if(!moveCard(CardZone::Hand, CardZone::MonsterZone, handIndex)) return false;
    normalSummonUsed = true;
    dispatcher.emit(GameEventType::NormalSummon); // evento specifico per la summon
    return true;
}

void Game::resetNormalSummon(){
    normalSummonUsed = false;
}

const std::vector<Card>& Game::getMonsterZone() const {
    return monsterZones[turn.getCurrentPlayerIndex()];
}

const std::vector<Card>& Game::getOpponentMonsterZone() const {
    return monsterZones[1 - turn.getCurrentPlayerIndex()];
}

std::vector<Card>& Game::getGraveyard(){
    return graveyard[0];
}

std::vector<Card>& Game::getOpponentGraveyard(){
    return graveyard[1];
}

void Game::discardExcess(size_t handLimit){
    Player& p = current();
    auto &hand = p.getHand();
    while(hand.size() > handLimit){
        // Sposta l'ultima carta (hand.size()-1) nel Cimitero tramite API (emette eventi)
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
    return extracted; // il chiamante deciderà quando inserirle nel Cimitero (post animazione)
}

void Game::advancePhase(){
    GamePhase before = turn.getPhase();
    turn.nextPhase();
    if(turn.getPhase() != before){
    // Reset della Normal Summon quando si entra in Draw o Standby
        if(turn.getPhase() == GamePhase::Draw || turn.getPhase() == GamePhase::Standby){
            resetNormalSummon();
        }
    dispatcher.emit(GameEventType::PhaseChange); // cambio fase generico
    // Pescata automatica quando si entra nella Draw Phase
        if(turn.getPhase() == GamePhase::Draw && drawCtrl){
            startTurn();
        }
        if(turn.getPhase() == GamePhase::Standby){
            processEnterStandby();
        }
            if(turn.getPhase() == GamePhase::Battle){
                dispatcher.emit(GameEventType::BattleStart);
            }
            if(turn.getPhase() == GamePhase::Main2 && before == GamePhase::Battle){
                dispatcher.emit(GameEventType::BattleEnd);
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
    return extractExcessCards(handLimit);
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
    // Chiamato dal layer esterno quando l'animazione di scarto termina -> fine turno immediata
    pendingAutoTurnEnd = false;
    endTurn();
}

void Game::processEnterEndPhase(){
    // Se la mano eccede il limite, estrai e delega l'animazione
    Player& p = current();
    auto &hand = p.getHand();
    if(hand.size() > handLimit){
        if(discardCallback){
            discardCallback(handleEndPhase(handLimit));
        } else {
            // Fallback immediato: scarta direttamente nel Cimitero senza animazione
            discardExcess(handLimit);
        }
    } else {
    // Nessun eccesso -> programma auto end turno (consumato dal main o immediato se si vuole)
        pendingAutoTurnEnd = true;
    }
}

void Game::processEnterStandby(){
    // Auto-skip della Standby se nessun effetto (placeholder: sempre skip per ora)
    bool shouldSkip = true; // TODO: interrogare effect system
    if(shouldSkip){
        dispatcher.emit(GameEventType::StandbySkip);
    // Avanza direttamente a Main1 (cambio di stato interno)
        turn.nextPhase();
        dispatcher.emit(GameEventType::PhaseChange);
    }
}

void Game::startTurn(){
    resetNormalSummon();
    dispatcher.emit(GameEventType::TurnStart);
    
    // Siamo nella Draw Phase impostata da TurnManager::endTurn() oppure all'inizio partita
    // Reset dei flag "ha attaccato" per il giocatore corrente
    {
        auto &flags = monsterHasAttacked[turn.getCurrentPlayerIndex()];
        std::fill(flags.begin(), flags.end(), false);
    }
    if(drawCtrl){
    // Il primissimo turno del giocatore iniziale non prevede la pescata
        bool mustDraw = !(firstTurn && turn.getCurrentPlayerIndex() == startingPlayerIndex);
        if(mustDraw){
            handleDeckOutIfAny();
            if(!gameOver){
                drawCtrl->queueDraw(1);
            }
        }
    }
}

bool Game::moveCard(CardZone from, CardZone to, size_t indexFrom){
    Player &p = current();
    std::vector<Card>* source = nullptr;
    std::vector<Card>* dest = nullptr;
    switch(from){
        case CardZone::Hand: source = &p.getHand(); break;
        case CardZone::MonsterZone: source = &monsterZones[turn.getCurrentPlayerIndex()]; break;
        case CardZone::Graveyard: source = &graveyard[turn.getCurrentPlayerIndex()]; break;
            case CardZone::Banished: source = &banished[turn.getCurrentPlayerIndex()]; break;
            case CardZone::Extra: /* gestito nel Deck/UI, non spostiamo qui per indice */ return false;
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
        case CardZone::Graveyard: dest = &graveyard[turn.getCurrentPlayerIndex()]; break;
            case CardZone::Banished: dest = &banished[turn.getCurrentPlayerIndex()]; break;
            case CardZone::Extra: return false; // Extra non gestito qui
            case CardZone::Deck: return false; // no reinsert deck for now
    }
    if(!dest) return false;
    dest->push_back(card);
    // Mantieni i vettori dei flag allineati quando aggiungi un mostro del giocatore corrente
    if(to == CardZone::MonsterZone){
        auto cur = turn.getCurrentPlayerIndex();
        monsterHasAttacked[cur].push_back(false);
    }
    dispatcher.emit(GameEventType::CardMoved);
    if(to == CardZone::Graveyard){
        dispatcher.emit(GameEventType::CardSentToGrave);
    }
    return true;
}

int Game::requiredTributesFor(const Card& c) const{
    auto lvl = c.getLevelOrRank();
    if(!lvl.has_value()) return 0;
    int L = *lvl;
    if(L <= 4) return 0;
    if(L >=5 && L <=6) return 1;
    if(L >=7) return 2;
    return 0;
}

bool Game::tributeMonsters(const std::vector<size_t>& zoneIndices){
    int idx = turn.getCurrentPlayerIndex();
    auto &zone = monsterZones[idx];
    if(zoneIndices.empty()) return true;
    // Verifica validità indici e nessuna duplicazione
    std::vector<size_t> sorted = zoneIndices;
    std::sort(sorted.begin(), sorted.end());
    if(std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end()) return false;
    if(sorted.back() >= zone.size()) return false;
    // Rimuovi dal fondo degli indici per non invalidare
    for(auto it = sorted.rbegin(); it != sorted.rend(); ++it){
        // sposta carta nel graveyard
        Card moved = zone[*it];
        zone.erase(zone.begin()+*it);
        graveyard[idx].push_back(moved);
        // allinea flags
        if(*it < monsterHasAttacked[idx].size()){
            monsterHasAttacked[idx].erase(monsterHasAttacked[idx].begin()+*it);
        }
    }
    dispatcher.emit(GameEventType::MonstersTributed);
    dispatcher.emit(GameEventType::CardSentToGrave); // generico (almeno uno)
    return true;
}

bool Game::beginNormalSummonWithTributes(size_t handIndex){
    if(pendingSummonHandIndex.has_value()) return false; // già in corso
    Player &p = current();
    auto &hand = p.getHand();
    if(handIndex >= hand.size()) return false;
    int needed = requiredTributesFor(hand[handIndex]);
    if(needed <= 0) return false; // non dovrebbe capitare
    if(monsterZones[turn.getCurrentPlayerIndex()].size() < static_cast<size_t>(needed)) return false; // non abbastanza mostri
    pendingSummonHandIndex = handIndex;
    pendingTributesNeeded = needed;
    dispatcher.emit(GameEventType::NormalSummonTributeRequired);
    return true; // segnale alla UI che deve raccogliere tributi
}

bool Game::completePendingNormalSummon(const std::vector<size_t>& tributeIndices){
    if(!pendingSummonHandIndex.has_value()) return false;
    if(static_cast<int>(tributeIndices.size()) != pendingTributesNeeded) return false;
    // Verifica e tributa
    if(!tributeMonsters(tributeIndices)) return false;
    size_t handIndex = *pendingSummonHandIndex;
    pendingSummonHandIndex.reset();
    pendingTributesNeeded = 0;
    // Attenzione: dopo i tributi la mano potrebbe essere cambiata? (no, tributi rimuovono solo dalla monster zone)
    if(!moveCard(CardZone::Hand, CardZone::MonsterZone, handIndex)) return false;
    normalSummonUsed = true;
    dispatcher.emit(GameEventType::NormalSummon);
    return true;
}

void Game::cancelPendingNormalSummon(){
    pendingSummonHandIndex.reset();
    pendingTributesNeeded = 0;
}

//================= Battle (v1) =================//

bool Game::canDeclareAttack(size_t attackerIndex, std::optional<size_t> targetIndex) const{
    if(firstTurn) return false; // nessun attacco nel primissimo turno
    if(turn.getPhase() != GamePhase::Battle) return false;
    int cur = turn.getCurrentPlayerIndex();
    if(attackerIndex >= monsterZones[cur].size()) return false;
    if(attackerIndex >= monsterHasAttacked[cur].size()) return false;
    if(monsterHasAttacked[cur][attackerIndex]) return false; // già usato

    int opp = 1 - cur;
    if(targetIndex.has_value()){
        return targetIndex.value() < monsterZones[opp].size();
    } else {
        return monsterZones[opp].empty();
    }
}

void Game::destroyMonster(int playerIdx, size_t zoneIndex){
    if(playerIdx < 0 || playerIdx > 1) return;
    if(zoneIndex >= monsterZones[playerIdx].size()) return;
    graveyard[playerIdx].push_back(monsterZones[playerIdx][zoneIndex]);
    monsterZones[playerIdx].erase(monsterZones[playerIdx].begin()+zoneIndex);
    if(zoneIndex < monsterHasAttacked[playerIdx].size()){
        monsterHasAttacked[playerIdx].erase(monsterHasAttacked[playerIdx].begin()+zoneIndex);
    }
    dispatcher.emit(GameEventType::CardSentToGrave);
    dispatcher.emit(GameEventType::MonsterDestroyed);
}

void Game::dealDamageTo(int playerIdx, int amount){
    if(amount <= 0) return;
    if(playerIdx < 0 || playerIdx > 1) return;
    players[playerIdx].damage(amount);
    dispatcher.emit(GameEventType::LifePointsChanged);
    checkVictoryByLP();
}

bool Game::declareAttack(size_t attackerIndex, std::optional<size_t> maybeTargetIndex){
    if(!canDeclareAttack(attackerIndex, maybeTargetIndex)) return false;
    int cur = turn.getCurrentPlayerIndex();
    int opp = 1 - cur;
    dispatcher.emit(GameEventType::AttackDeclared);
    dispatchEffects(GameEventType::AttackDeclared);

    if(attackerIndex >= monsterZones[cur].size()) return false;
    int atkA = 0;
    if(monsterZones[cur][attackerIndex].getValues().has_value()){
        atkA = monsterZones[cur][attackerIndex].getValues().value().first;
    }

    if(maybeTargetIndex.has_value()){
        size_t tIdx = maybeTargetIndex.value();
        if(tIdx >= monsterZones[opp].size()) return false;
        int atkB = 0;
        if(monsterZones[opp][tIdx].getValues().has_value()){
            atkB = monsterZones[opp][tIdx].getValues().value().first;
        }
        if(atkA > atkB){
            destroyMonster(opp, tIdx);
            dealDamageTo(opp, atkA - atkB);
            if(attackerIndex < monsterHasAttacked[cur].size()) monsterHasAttacked[cur][attackerIndex] = true;
        } else if(atkA < atkB){
            destroyMonster(cur, attackerIndex);
            dealDamageTo(cur, atkB - atkA);
        } else {
            // pareggio: distruggi entrambi
            // distruggi prima quello con indice maggiore per preservare indici
            if(attackerIndex > tIdx){
                destroyMonster(cur, attackerIndex);
                destroyMonster(opp, tIdx);
            } else {
                destroyMonster(opp, tIdx);
                destroyMonster(cur, attackerIndex);
            }
        }
    } else {
        // attacco diretto
        dealDamageTo(opp, atkA);
        if(attackerIndex < monsterHasAttacked[cur].size()) monsterHasAttacked[cur][attackerIndex] = true;
        dispatcher.emit(GameEventType::DirectAttack);
    }
    dispatcher.emit(GameEventType::AttackResolved);
    dispatchEffects(GameEventType::AttackResolved);
    return true;
}

void Game::debugAddMonsterToOpponent(const Card& c){
    int opp = 1 - turn.getCurrentPlayerIndex();
    if(monsterZones[opp].size() >= MONSTER_ZONE_SIZE) return;
    monsterZones[opp].push_back(c);
    if(monsterHasAttacked[opp].size() < monsterZones[opp].size()){
        monsterHasAttacked[opp].push_back(false);
    }
    dispatcher.emit(GameEventType::CardMoved);
}

void Game::checkVictoryByLP(){
    if(gameOver) return;
    for(int i=0;i<2;++i){
        if(players[i].getLifePoints() <= 0){
            gameOver = true;
            winnerIndex = 1 - i;
            dispatcher.emit(GameEventType::Win);
            dispatcher.emit(GameEventType::Lose);
            break;
        }
    }
}

void Game::handleDeckOutIfAny(){
    if(gameOver) return;
    int cur = turn.getCurrentPlayerIndex();
    bool logicalCantDraw = !current().canDraw();
    bool uiDeckEmpty = (externalDeck && externalDeck->isEmpty());
    if(logicalCantDraw || uiDeckEmpty){
        gameOver = true;
        winnerIndex = 1 - cur;
        dispatcher.emit(GameEventType::Win);
        dispatcher.emit(GameEventType::Lose);
    }
}

bool Game::hasMonsterAlreadyAttacked(size_t zoneIndex) const{
    int cur = turn.getCurrentPlayerIndex();
    if(zoneIndex >= monsterHasAttacked[cur].size()) return false;
    return monsterHasAttacked[cur][zoneIndex];
}

bool Game::banishFrom(CardZone from, size_t indexFrom){
        return moveCard(from, CardZone::Banished, indexFrom);
}        

// Propagazione effetti tramite sistema dedicato
void Game::dispatchEffects(GameEventType type){
    effects.dispatch(type, *this);
}

void Game::registerEffectForCardName(const std::string& cardName, std::unique_ptr<ICardEffect> effect){
    effects.registerEffectForCardName(cardName, std::move(effect));
}
