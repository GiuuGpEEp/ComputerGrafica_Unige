#include "Game.h"
#include "../DrawController/DrawController.h"
#include "Effects/ICardEffect.h"
#include "Effects/EffectSystem.h"
#include "Effects/IActivationEffect.h"
#include <algorithm>
#include <iostream>

Game::Game(const Player& p1, const Player& p2)
    : players{p1, p2} {
        for(auto& mz : monsterZones) mz.reserve(MONSTER_ZONE_SIZE);
    // slot mapping init
    monsterSlotIndex[0].clear();
    monsterSlotIndex[1].clear();
    for(auto& st : spellTraps) st.reserve(ST_ZONE_SIZE);
    // Nessun listener interno al TurnManager: tutta la propagazione passa da EventDispatcher
    monsterHasAttacked[0].clear();
    monsterHasAttacked[1].clear();
    monsterIsDefense[0].clear();
    monsterIsDefense[1].clear();
    monsterIsFaceDown[0].clear();
    monsterIsFaceDown[1].clear();
    monsterSummonedThisTurn[0].clear();
    monsterSummonedThisTurn[1].clear();
    monsterPositionChangedThisTurn[0].clear();
    monsterPositionChangedThisTurn[1].clear();
    stIsFaceDown[0].clear();
    stIsFaceDown[1].clear();
    stSetThisTurn[0].clear();
    stSetThisTurn[1].clear();
    fieldSpell[0].reset();
    fieldSpell[1].reset();
        banished[0].clear();
        banished[1].clear();

    // Forward automatico verso il sistema effetti per alcuni eventi non ancora inoltrati direttamente
    // Nota: evitiamo di duplicare eventi già inoltrati esplicitamente (TurnStart, PhaseChange, BattleStart/End, AttackDeclared/Resolved)
    dispatcher.subscribe(GameEventType::NormalSummon, [this]() { this->dispatchEffects(GameEventType::NormalSummon); });
    dispatcher.subscribe(GameEventType::MonstersTributed, [this]() { this->dispatchEffects(GameEventType::MonstersTributed); });
    dispatcher.subscribe(GameEventType::CardSentToGrave, [this]() { this->dispatchEffects(GameEventType::CardSentToGrave); });
    dispatcher.subscribe(GameEventType::PhaseChange, [this]() { this->dispatchEffects(GameEventType::PhaseChange); });
    dispatcher.subscribe(GameEventType::TurnStart, [this]() { this->dispatchEffects(GameEventType::TurnStart); });
    dispatcher.subscribe(GameEventType::LifePointsChanged, [this]() { this->dispatchEffects(GameEventType::LifePointsChanged); });
    dispatcher.subscribe(GameEventType::DirectAttack, [this]() { this->dispatchEffects(GameEventType::DirectAttack); });
    dispatcher.subscribe(GameEventType::MonsterFlipped, [this]() { this->dispatchEffects(GameEventType::MonsterFlipped); });
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
    // Niente distribuzione mano iniziale qui: viene gestita a livello UI con animazione alternata.
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
    // Disponibile se c'è almeno uno slot libero
    int cur = turn.getCurrentPlayerIndex();
    return firstFreeMonsterSlot(cur).has_value();
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
        return beginNormalSummonWithTributes(handIndex, /*asSet*/false);
    }
    // Nessun tributo richiesto: serve uno slot libero
    int curIdx = turn.getCurrentPlayerIndex();
    auto freeSlot = firstFreeMonsterSlot(curIdx);
    if(!freeSlot.has_value()) return false;
    if(!tryNormalSummonToSlot(handIndex, *freeSlot)) return false;
    // Mark flags for newly summoned monster
    {
        int cur = turn.getCurrentPlayerIndex();
        if(!monsterSummonedThisTurn[cur].empty()) monsterSummonedThisTurn[cur].back() = true;
        if(!monsterPositionChangedThisTurn[cur].empty()) monsterPositionChangedThisTurn[cur].back() = false;
    }
    normalSummonUsed = true;
    dispatcher.emit(GameEventType::NormalSummon); // evento specifico per la summon
    return true;
}

bool Game::tryNormalSummonToSlot(size_t handIndex, size_t slotIndex){
    Player& p = current();
    auto& hand = p.getHand();
    if(handIndex >= hand.size()) return false;
    if(normalSummonUsed) return false;
    if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
    int tributesNeeded = 0;
    if(auto lvl = hand[handIndex].getLevelOrRank(); lvl.has_value()){
        tributesNeeded = requiredTributesFor(hand[handIndex]);
    }
    int cur = turn.getCurrentPlayerIndex();
    if(tributesNeeded > 0){
        // Salva slot e avvia flusso tributi
        pendingSummonSlotIndex = slotIndex;
        return beginNormalSummonWithTributes(handIndex, /*asSet*/false, pendingSummonSlotIndex);
    }
    // Nessun tributo: verifica slot libero specifico
    if(slotIndex >= MONSTER_ZONE_SIZE) return false;
    if(isMonsterSlotOccupied(cur, slotIndex)) return false;
    // Esegui move e registra slot mapping
    if(!moveCard(CardZone::Hand, CardZone::MonsterZone, handIndex)) return false;
    if(!monsterSlotIndex[cur].empty()) monsterSlotIndex[cur].back() = static_cast<int>(slotIndex);
    // Mark flags gia' fatti in moveCard per summoned/positionChanged
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

const std::vector<Card>& Game::getOpponentMonsterZone() const {
    return monsterZones[1 - turn.getCurrentPlayerIndex()];
}

std::vector<Card>& Game::getGraveyard(){
    return graveyard[turn.getCurrentPlayerIndex()];
}

std::vector<Card>& Game::getOpponentGraveyard(){
    return graveyard[1 - turn.getCurrentPlayerIndex()];
}

std::vector<Card>& Game::getGraveyardOf(int playerIdx){
    if(playerIdx < 0 || playerIdx > 1) return graveyard[0];
    return graveyard[playerIdx];
}

std::vector<Card>& Game::getHandOf(int playerIdx){
    if(playerIdx < 0 || playerIdx > 1) return players[0].getHand();
    return players[playerIdx].getHand();
}

const Deck& Game::getDeckOf(int playerIdx) const{
    if(playerIdx < 0 || playerIdx > 1) return players[0].getDeck();
    return players[playerIdx].getDeck();
}

Deck& Game::getDeckOf(int playerIdx){
    if(playerIdx < 0 || playerIdx > 1) return const_cast<Deck&>(players[0].getDeck());
    return const_cast<Deck&>(players[playerIdx].getDeck());
}

void Game::addToHand(int playerIdx, Card&& c){
    if(playerIdx < 0 || playerIdx > 1) return;
    players[playerIdx].getHand().push_back(std::move(c));
    dispatcher.emit(GameEventType::CardMoved);
}

void Game::discardExcess(size_t handLimit){
    Player& p = current();
    auto &hand = p.getHand();
    while(hand.size() > handLimit){
        // Sposta l'ultima carta (hand.size()-1) nel Cimitero tramite API (emette eventi)
        moveCard(CardZone::Hand, CardZone::Graveyard, hand.size()-1);
    }
}

void Game::mirrorExternalDeckRemoveByName(const std::string& name, const Deck* sourceDeck){
    // Se non sono collegati deck esterni, nulla da sincronizzare
    bool any = (externalDecks[0] != nullptr) || (externalDecks[1] != nullptr);
    if(!any) return;
    // Try to find which external deck corresponds to the source (if provided)
    if(sourceDeck){
        for(int i=0;i<2;++i){
            if(externalDecks[i] && externalDecks[i] == sourceDeck){
                // Stessa istanza: si assume che la UI rifletta già la rimozione o che sarà guidata dalla stessa reference
                return;
            }
        }
    }
    // Altrimenti, rimuovi in modo conservativo da entrambi i deck esterni (solo visuale)
    for(int i=0;i<2;++i){
        if(externalDecks[i]){
            auto removed = externalDecks[i]->removeFirstByName(name);
            (void)removed;
        }
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

void Game::attachExternalDeck(Deck* deckPtr, int playerIdx){
    if(playerIdx < 0 || playerIdx > 1) return;
    externalDecks[playerIdx] = deckPtr;
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

bool Game::shouldAutoEndTurn() const { return pendingAutoTurnEnd && !pendingSS.has_value(); }

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
        // Nessun eccesso -> programma auto end turno, ma solo se non c'è una scelta di Evocazione Speciale in sospeso
        pendingAutoTurnEnd = !pendingSS.has_value();
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
    // Reset dei flag temporanei di turno
    negateOppMonsterEffUntilEndOfTurn[0] = false;
    negateOppMonsterEffUntilEndOfTurn[1] = false;
    // Siamo nella Draw Phase impostata da TurnManager::endTurn() oppure all'inizio partita
    // Reset dei flag "ha attaccato" per il giocatore corrente
    {
        auto &flags = monsterHasAttacked[turn.getCurrentPlayerIndex()];
        std::fill(flags.begin(), flags.end(), false);
    }
    // Reset position-changed flag; clear summoned-this-turn from previous turn for CURRENT player's monsters
    {
        int cur = turn.getCurrentPlayerIndex();
        auto &posChanged = monsterPositionChangedThisTurn[cur];
        std::fill(posChanged.begin(), posChanged.end(), false);
        auto &summoned = monsterSummonedThisTurn[cur];
        std::fill(summoned.begin(), summoned.end(), false);
    // Le carte M/T settate nel turno PRECEDENTE (cioè del giocatore avversario rispetto a quello corrente)
    // diventano attivabili: bisogna azzerare il flag "settata in questo turno" per il giocatore che ha appena
    // concluso il turno, non per quello che lo inizia. In questo modo una Trap/Magia Rapida settata nel proprio
    // turno è attivabile già nel turno successivo dell'avversario, come da regola.
    int prev = 1 - cur;
    auto &stSetPrev = stSetThisTurn[prev];
    std::fill(stSetPrev.begin(), stSetPrev.end(), false);
    }
    if(drawCtrl){
        int cur = turn.getCurrentPlayerIndex();
        if(firstTurn){
            // Il giocatore iniziale NON pesca nella sua prima Draw; l'altro riceve una mano iniziale di 5 carte
            if(cur == startingPlayerIndex){
                // nessuna pescata
            } else {
                // Evita doppia mano iniziale: se ha già >=5 carte (già data all'avvio), non accodare di nuovo
                if(players[cur].getHand().size() < 5){
                    handleDeckOutIfAny();
                    if(!gameOver){
                        drawCtrl->queueDraw(5);
                    }
                }
            }
        } else {
            // Turni successivi: pescata singola standard
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
    bool sourceIsMonsterZone = false;
    bool sourceIsSTZone = false;
    switch(from){
        case CardZone::Hand: source = &p.getHand(); break;
        case CardZone::MonsterZone: source = &monsterZones[turn.getCurrentPlayerIndex()]; sourceIsMonsterZone = true; break;
        case CardZone::SpellTrapZone: source = &spellTraps[turn.getCurrentPlayerIndex()]; sourceIsSTZone = true; break;
        case CardZone::FieldSpellZone: /* gestito a parte: singolo slot, non rimuovibile per indice qui */ return false;
        case CardZone::Graveyard: source = &graveyard[turn.getCurrentPlayerIndex()]; break;
            case CardZone::Banished: source = &banished[turn.getCurrentPlayerIndex()]; break;
            case CardZone::Extra: /* gestito nel Deck/UI, non spostiamo qui per indice */ return false;
        case CardZone::Deck: /* non supportato estrarre per indice diretto */ return false;
    }
    if(!source) return false;
    if(indexFrom >= source->size()) return false;
    // Se stiamo rimuovendo dalla MonsterZone, allinea i vettori di flag prima di rimuovere
    if(sourceIsMonsterZone){
        int cur = turn.getCurrentPlayerIndex();
        if(indexFrom < monsterHasAttacked[cur].size()){
            monsterHasAttacked[cur].erase(monsterHasAttacked[cur].begin()+indexFrom);
        }
        if(indexFrom < monsterIsDefense[cur].size()){
            monsterIsDefense[cur].erase(monsterIsDefense[cur].begin()+indexFrom);
        }
        if(indexFrom < monsterIsFaceDown[cur].size()){
            monsterIsFaceDown[cur].erase(monsterIsFaceDown[cur].begin()+indexFrom);
        }
        if(indexFrom < monsterSummonedThisTurn[cur].size()){
            monsterSummonedThisTurn[cur].erase(monsterSummonedThisTurn[cur].begin()+indexFrom);
        }
        if(indexFrom < monsterPositionChangedThisTurn[cur].size()){
            monsterPositionChangedThisTurn[cur].erase(monsterPositionChangedThisTurn[cur].begin()+indexFrom);
        }
    }
    if(sourceIsSTZone){
        int cur = turn.getCurrentPlayerIndex();
        if(indexFrom < stIsFaceDown[cur].size()){
            stIsFaceDown[cur].erase(stIsFaceDown[cur].begin()+indexFrom);
        }
        if(indexFrom < stSetThisTurn[cur].size()){
            stSetThisTurn[cur].erase(stSetThisTurn[cur].begin()+indexFrom);
        }
    }
    Card card = (*source)[indexFrom];
    source->erase(source->begin()+indexFrom);
    // destinazione
    switch(to){
        case CardZone::Hand: dest = &p.getHand(); break;
    case CardZone::MonsterZone:
            dest = &monsterZones[turn.getCurrentPlayerIndex()];
            if(dest->size() >= MONSTER_ZONE_SIZE){ return false; }
            break;
        case CardZone::SpellTrapZone:
            dest = &spellTraps[turn.getCurrentPlayerIndex()];
            if(dest->size() >= ST_ZONE_SIZE){ return false; }
            break;
        case CardZone::Graveyard: dest = &graveyard[turn.getCurrentPlayerIndex()]; break;
            case CardZone::Banished: dest = &banished[turn.getCurrentPlayerIndex()]; break;
            case CardZone::Extra: return false; // Extra non gestito qui
            case CardZone::Deck: return false; // no reinsert deck for now
        case CardZone::FieldSpellZone: return false; // usa API dedicate
    }
    if(!dest) return false;
    dest->push_back(card);
    // Mantieni i vettori dei flag allineati quando aggiungi un mostro del giocatore corrente
    if(to == CardZone::MonsterZone){
        auto cur = turn.getCurrentPlayerIndex();
        monsterHasAttacked[cur].push_back(false);
        monsterIsDefense[cur].push_back(false);
        monsterIsFaceDown[cur].push_back(false);
        // Mark as newly summoned this turn; position-change not yet used
        monsterSummonedThisTurn[cur].push_back(true);
        monsterPositionChangedThisTurn[cur].push_back(false);
    // Placeholder slot index, verra' impostato dal chiamante se necessario
    monsterSlotIndex[cur].push_back(-1);
    }
    if(to == CardZone::SpellTrapZone){
        auto cur = turn.getCurrentPlayerIndex();
        stIsFaceDown[cur].push_back(true); // set di default coperto
        stSetThisTurn[cur].push_back(true);
    }
    dispatcher.emit(GameEventType::CardMoved);
    if(to == CardZone::Graveyard){
    lastSentToGrave = LastSentToGrave{ card.getName(), turn.getCurrentPlayerIndex() };
    std::cout << "[DEBUG] Game::moveCard - lastSentToGrave set name='" << lastSentToGrave->name << "' owner=" << lastSentToGrave->ownerIdx << " (moveCard)" << std::endl;
        dispatcher.emit(GameEventType::CardSentToGrave);
    }
    return true;
}

// =====================
// Scelte dal Deck e dalla Mano (Reliquiario / Melodia)
// =====================

bool Game::requestSendFromDeck(int playerIdx){
    if(pendingSend.has_value() || pendingDiscard.has_value() || pendingAdd.has_value()) return false;
    if(playerIdx < 0 || playerIdx > 1) return false;
    pendingSend = PendingSendDeck{ playerIdx };
    dispatcher.emit(GameEventType::DeckSendChoiceRequested);
    return true;
}

std::optional<int> Game::getPendingSendOwner() const{
    if(!pendingSend.has_value()) return std::nullopt;
    return pendingSend->ownerIdx;
}

bool Game::resolvePendingSendFromDeck(size_t indexInFiltered){
    if(!pendingSend.has_value()) return false;
    int owner = pendingSend->ownerIdx;
    Deck &d = getDeckOf(owner);
    // Ricostruisci la lista filtrata (Drago) come fa la UI
    auto candidates = d.collectWhere([](const Card& c){
        const auto &feats = c.getFeatures();
        return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
    });
    if(indexInFiltered >= candidates.size()) return false;
    const std::string pickName = candidates[indexInFiltered].getName();
    auto picked = d.removeFirstByName(pickName);
    if(!picked.has_value()) return false;
    // Invia al Cimitero del proprietario
    auto &gy = getGraveyardOf(owner);
    gy.push_back(std::move(*picked));
    lastSentToGrave = Game::LastSentToGrave{ gy.back().getName(), owner };
    dispatcher.emit(GameEventType::CardSentToGrave);
    dispatcher.emit(GameEventType::CardMoved);
    // Sincronizza eventuali deck esterni
    mirrorExternalDeckRemoveByName(pickName, &d);
    pendingSend.reset();
    return true;
}

void Game::cancelPendingSendFromDeck(){ pendingSend.reset(); }

// ---- Nuove API: scelta scarto dalla mano ----
bool Game::requestHandDiscard(int playerIdx){
    if(pendingSend.has_value() || pendingDiscard.has_value() || pendingAdd.has_value()) return false;
    if(playerIdx < 0 || playerIdx > 1) return false;
    auto &hand = getHandOf(playerIdx);
    if(hand.empty()) return false;
    pendingDiscard = PendingDiscardHand{ playerIdx };
    dispatcher.emit(GameEventType::HandDiscardChoiceRequested);
    return true;
}

std::optional<int> Game::getPendingHandDiscardOwner() const{
    if(!pendingDiscard.has_value()) return std::nullopt;
    return pendingDiscard->ownerIdx;
}

bool Game::resolvePendingHandDiscard(size_t handIndex){
    if(!pendingDiscard.has_value()) return false;
    int owner = pendingDiscard->ownerIdx;
    auto &hand = getHandOf(owner);
    if(handIndex >= hand.size()) return false;
    // Sposta in Cimitero
    Card discarded = std::move(hand[handIndex]);
    hand.erase(hand.begin() + handIndex);
    auto &gy = getGraveyardOf(owner);
    gy.push_back(std::move(discarded));
    lastSentToGrave = Game::LastSentToGrave{ gy.back().getName(), owner };
    dispatcher.emit(GameEventType::CardSentToGrave);
    dispatcher.emit(GameEventType::CardMoved);
    pendingDiscard.reset();
    // Se è impostato un passo successivo (es. Melodia -> aggiungi dal Deck), eseguilo ora
    if(nextAddAfterDiscardOwner.has_value()){
        int addOwner = *nextAddAfterDiscardOwner;
        nextAddAfterDiscardOwner.reset();
        // Avvia richiesta di aggiunta dal Deck (filtrata per Melodia)
        (void)requestAddFromDeck(addOwner);
    }
    return true;
}

void Game::cancelPendingHandDiscard(){ pendingDiscard.reset(); }

// ---- Nuove API: scelta aggiunta dal Deck alla mano ----
namespace { // file-scope helper per filtro Melodia
    static bool melodiaFilter(const Card& c){
        auto val = c.getValues();
        if(!val.has_value()) return false;
        int atk = val->first;
        int def = val->second;
        if(atk < 3000 || def > 2500) return false;
        const auto& feats = c.getFeatures();
        return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
    }

bool Game::requestAddFromDeck(int playerIdx){
    if(pendingSend.has_value() || pendingDiscard.has_value() || pendingAdd.has_value()) return false;
    if(playerIdx < 0 || playerIdx > 1) return false;
    // Verifica esistenza almeno di un candidato
    const Deck &d = getDeckOf(playerIdx);
    auto candidates = d.collectWhere(melodiaFilter);
    if(candidates.empty()) return false;
    pendingAdd = PendingAddDeck{ playerIdx };
    dispatcher.emit(GameEventType::DeckAddChoiceRequested);
    return true;
}

std::optional<int> Game::getPendingAddOwner() const{
    if(!pendingAdd.has_value()) return std::nullopt;
    return pendingAdd->ownerIdx;
}

bool Game::resolvePendingAddFromDeck(size_t indexInFiltered){
    if(!pendingAdd.has_value()) return false;
    int owner = pendingAdd->ownerIdx;
    Deck &d = getDeckOf(owner);
    auto candidates = d.collectWhere(melodiaFilter);
    if(indexInFiltered >= candidates.size()) return false;
    const std::string pickName = candidates[indexInFiltered].getName();
    auto picked = d.removeFirstByName(pickName);
    if(!picked.has_value()) return false;
    mirrorExternalDeckRemoveByName(pickName, &d);
    addToHand(owner, std::move(*picked));
    pendingAdd.reset();
    return true;
}

void Game::cancelPendingAddFromDeck(){ pendingAdd.reset(); }

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
        lastSentToGrave = LastSentToGrave{ graveyard[idx].back().getName(), idx };
        std::cout << "[DEBUG] Game::tributeMonsters - lastSentToGrave name='" << lastSentToGrave->name << "' owner=" << lastSentToGrave->ownerIdx << " (tributeMonsters)" << std::endl;
        // allinea flags e slot mapping
        if(*it < monsterHasAttacked[idx].size()){
            monsterHasAttacked[idx].erase(monsterHasAttacked[idx].begin()+*it);
        }
        if(*it < monsterIsDefense[idx].size()){
            monsterIsDefense[idx].erase(monsterIsDefense[idx].begin()+*it);
        }
        if(*it < monsterIsFaceDown[idx].size()){
            monsterIsFaceDown[idx].erase(monsterIsFaceDown[idx].begin()+*it);
        }
        if(*it < monsterSummonedThisTurn[idx].size()){
            monsterSummonedThisTurn[idx].erase(monsterSummonedThisTurn[idx].begin()+*it);
        }
        if(*it < monsterPositionChangedThisTurn[idx].size()){
            monsterPositionChangedThisTurn[idx].erase(monsterPositionChangedThisTurn[idx].begin()+*it);
        }
        if(*it < monsterSlotIndex[idx].size()){
            monsterSlotIndex[idx].erase(monsterSlotIndex[idx].begin()+*it);
        }
    }
    dispatcher.emit(GameEventType::MonstersTributed);
    dispatcher.emit(GameEventType::CardSentToGrave); // generico (almeno uno)
    return true;
}

bool Game::beginNormalSummonWithTributes(size_t handIndex){
    return beginNormalSummonWithTributes(handIndex, /*asSet*/false);
}

bool Game::beginNormalSummonWithTributes(size_t handIndex, bool asSet){
    if(pendingSummonHandIndex.has_value()) return false; // già in corso
    Player &p = current();
    auto &hand = p.getHand();
    if(handIndex >= hand.size()) return false;
    int needed = requiredTributesFor(hand[handIndex]);
    if(needed <= 0) return false; // non dovrebbe capitare
    if(monsterZones[turn.getCurrentPlayerIndex()].size() < static_cast<size_t>(needed)) return false; // non abbastanza mostri
    pendingSummonHandIndex = handIndex;
    pendingSummonIsSet = asSet;
    pendingTributesNeeded = needed;
    dispatcher.emit(GameEventType::NormalSummonTributeRequired);
    return true; // segnale alla UI che deve raccogliere tributi
}

// Overload interno con memorizzazione slot scelto
bool Game::beginNormalSummonWithTributes(size_t handIndex, bool asSet, std::optional<size_t> slotIndexOpt){
    if(!beginNormalSummonWithTributes(handIndex, asSet)) return false;
    pendingSummonSlotIndex = slotIndexOpt;
    return true;
}

bool Game::completePendingNormalSummon(const std::vector<size_t>& tributeIndices){
    if(!pendingSummonHandIndex.has_value()) return false;
    if(static_cast<int>(tributeIndices.size()) != pendingTributesNeeded) return false;
    // Verifica e tributa
    if(!tributeMonsters(tributeIndices)) return false;
    size_t handIndex = *pendingSummonHandIndex;
    pendingSummonHandIndex.reset();
    pendingTributesNeeded = 0;
    bool asSet = pendingSummonIsSet;
    pendingSummonIsSet = false;
    // Attenzione: dopo i tributi la mano potrebbe essere cambiata? (no, tributi rimuovono solo dalla monster zone)
    if(!moveCard(CardZone::Hand, CardZone::MonsterZone, handIndex)) return false;
    int cur = turn.getCurrentPlayerIndex();
    // Mark summoned/position-changed flags for the newly placed monster
    if(!monsterSummonedThisTurn[cur].empty()) monsterSummonedThisTurn[cur].back() = true;
    if(!monsterPositionChangedThisTurn[cur].empty()) monsterPositionChangedThisTurn[cur].back() = false;
    // Assegna uno slot valido:
    // - se lo slot scelto dall'utente è ancora occupato dopo i tributi, usa il primo slot libero
    size_t chosen = 0;
    if(pendingSummonSlotIndex.has_value()){
        size_t candidate = *pendingSummonSlotIndex;
        pendingSummonSlotIndex.reset();
        if(candidate < MONSTER_ZONE_SIZE && !isMonsterSlotOccupied(cur, candidate)){
            chosen = candidate;
        } else {
            auto freeSlot = firstFreeMonsterSlot(cur);
            if(!freeSlot.has_value()){
                // Non dovrebbe accadere dopo aver tributato, ma fallback sicuro
                return false;
            }
            chosen = *freeSlot;
        }
    } else {
        auto freeSlot = firstFreeMonsterSlot(cur);
        if(!freeSlot.has_value()){
            // Non dovrebbe accadere dopo aver tributato
            return false;
        }
        chosen = *freeSlot;
    }
    if(!monsterSlotIndex[cur].empty()) monsterSlotIndex[cur].back() = static_cast<int>(chosen);
    if(asSet){
        if(!monsterIsDefense[cur].empty()) monsterIsDefense[cur].back() = true;
        if(!monsterIsFaceDown[cur].empty()) monsterIsFaceDown[cur].back() = true;
        dispatcher.emit(GameEventType::NormalSet);
    } else {
        dispatcher.emit(GameEventType::NormalSummon);
    }
    normalSummonUsed = true;
    return true;
}

void Game::cancelPendingNormalSummon(){
    pendingSummonHandIndex.reset();
    pendingTributesNeeded = 0;
    pendingSummonIsSet = false;
    pendingSummonSlotIndex.reset();
}

//================= Battle (v1) =================//

bool Game::canDeclareAttack(size_t attackerIndex, std::optional<size_t> targetIndex) const{
    if(firstTurn) return false; // nessun attacco nel primissimo turno
    if(turn.getPhase() != GamePhase::Battle) return false;
    int cur = turn.getCurrentPlayerIndex();
    if(attackerIndex >= monsterZones[cur].size()) return false;
    if(attackerIndex >= monsterHasAttacked[cur].size()) return false;
    if(monsterHasAttacked[cur][attackerIndex]) return false; // già usato
    // non può attaccare se in difesa o coperto
    if(attackerIndex < monsterIsDefense[cur].size() && monsterIsDefense[cur][attackerIndex]) return false;
    if(attackerIndex < monsterIsFaceDown[cur].size() && monsterIsFaceDown[cur][attackerIndex]) return false;

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
    lastSentToGrave = LastSentToGrave{ graveyard[playerIdx].back().getName(), playerIdx };
    monsterZones[playerIdx].erase(monsterZones[playerIdx].begin()+zoneIndex);
    if(zoneIndex < monsterHasAttacked[playerIdx].size()){
        monsterHasAttacked[playerIdx].erase(monsterHasAttacked[playerIdx].begin()+zoneIndex);
    }
    if(zoneIndex < monsterIsDefense[playerIdx].size()){
        monsterIsDefense[playerIdx].erase(monsterIsDefense[playerIdx].begin()+zoneIndex);
    }
    if(zoneIndex < monsterIsFaceDown[playerIdx].size()){
        monsterIsFaceDown[playerIdx].erase(monsterIsFaceDown[playerIdx].begin()+zoneIndex);
    }
    if(zoneIndex < monsterSummonedThisTurn[playerIdx].size()){
        monsterSummonedThisTurn[playerIdx].erase(monsterSummonedThisTurn[playerIdx].begin()+zoneIndex);
    }
    if(zoneIndex < monsterPositionChangedThisTurn[playerIdx].size()){
        monsterPositionChangedThisTurn[playerIdx].erase(monsterPositionChangedThisTurn[playerIdx].begin()+zoneIndex);
    }
    if(zoneIndex < monsterSlotIndex[playerIdx].size()){
        monsterSlotIndex[playerIdx].erase(monsterSlotIndex[playerIdx].begin()+zoneIndex);
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

    // Marca SUBITO il mostro come "ha attaccato" per evitare che uno shift del vettore
    // (in caso di distruzione) marchi il mostro sbagliato
    if(attackerIndex < monsterHasAttacked[cur].size()){
        monsterHasAttacked[cur][attackerIndex] = true;
    }

    if(attackerIndex >= monsterZones[cur].size()) return false;
    int atkA = 0;
    if(monsterZones[cur][attackerIndex].getValues().has_value()){
        atkA = monsterZones[cur][attackerIndex].getValues().value().first;
    }

    if(maybeTargetIndex.has_value()){
        size_t tIdx = maybeTargetIndex.value();
        if(tIdx >= monsterZones[opp].size()) return false;

        // Se il bersaglio è coperto, scoprirlo prima della risoluzione
        bool targetWasFaceDown = (tIdx < monsterIsFaceDown[opp].size() && monsterIsFaceDown[opp][tIdx]);
        if(targetWasFaceDown){
            monsterIsFaceDown[opp][tIdx] = false; // flip face-up
            // Nota: la posizione resta quella corrente (difesa o attacco)
            dispatcher.emit(GameEventType::CardMoved); // aggiorna rendering
            dispatcher.emit(GameEventType::MonsterFlipped);
        }

        bool targetIsDefense = (tIdx < monsterIsDefense[opp].size() && monsterIsDefense[opp][tIdx]);
        int atkOrDefB = 0;
        if(monsterZones[opp][tIdx].getValues().has_value()){
            if(targetIsDefense){
                atkOrDefB = monsterZones[opp][tIdx].getValues().value().second; // DEF
            } else {
                atkOrDefB = monsterZones[opp][tIdx].getValues().value().first; // ATK
            }
        }

    if(!targetIsDefense){
            // ATK vs ATK: come prima
            if(atkA > atkOrDefB){
                destroyMonster(opp, tIdx);
                dealDamageTo(opp, atkA - atkOrDefB);
            } else if(atkA < atkOrDefB){
                destroyMonster(cur, attackerIndex);
                dealDamageTo(cur, atkOrDefB - atkA);
            } else {
                // pareggio ATK: nessun danno; entrambi distrutti secondo regola attuale
                if(attackerIndex > tIdx){
                    destroyMonster(cur, attackerIndex);
                    destroyMonster(opp, tIdx);
                } else {
                    destroyMonster(opp, tIdx);
                    destroyMonster(cur, attackerIndex);
                }
            }
    } else {
            // ATK vs DEF (no piercing):
            if(atkA > atkOrDefB){
                // Distruggi solo il difensore, nessun danno al controllore del difensore
                destroyMonster(opp, tIdx);
            } else if(atkA < atkOrDefB){
        // L'attaccante NON viene distrutto; il suo controllore subisce la differenza
                dealDamageTo(cur, atkOrDefB - atkA);
            } else {
                // ATK == DEF: nessuna distruzione, nessun danno
            }
        }
    } else {
        // attacco diretto
        dealDamageTo(opp, atkA);
        dispatcher.emit(GameEventType::DirectAttack);
    }
    dispatcher.emit(GameEventType::AttackResolved);
    return true;
}

void Game::debugAddMonsterToOpponent(const Card& c){
    int opp = 1 - turn.getCurrentPlayerIndex();
    if(monsterZones[opp].size() >= MONSTER_ZONE_SIZE) return;
    monsterZones[opp].push_back(c);
    if(monsterHasAttacked[opp].size() < monsterZones[opp].size()){
        monsterHasAttacked[opp].push_back(false);
    }
    if(monsterIsDefense[opp].size() < monsterZones[opp].size()){
        monsterIsDefense[opp].push_back(false);
    }
    if(monsterIsFaceDown[opp].size() < monsterZones[opp].size()){
        monsterIsFaceDown[opp].push_back(false);
    }
    if(monsterSummonedThisTurn[opp].size() < monsterZones[opp].size()){
        // Considera i mostri inseriti via debug come non evocati in questo turno
        monsterSummonedThisTurn[opp].push_back(false);
    }
    if(monsterPositionChangedThisTurn[opp].size() < monsterZones[opp].size()){
        monsterPositionChangedThisTurn[opp].push_back(false);
    }
    // assegna uno slot libero
    auto slotOpt = firstFreeMonsterSlot(opp);
    monsterSlotIndex[opp].push_back(slotOpt.has_value()? static_cast<int>(*slotOpt) : -1);
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
    // Decide il deck out basandosi sulla logica di gioco (se il giocatore corrente non puo' pescare)
    // Evitiamo di affidarsi a `externalDeck` (rappresentazione UI) perché potrebbe non essere
    // aggiornato per il giocatore corrente e portare a falsi positivi.
    if(!current().canDraw()){
        gameOver = true;
        winnerIndex = 1 - cur; // il vincitore e' l'altro giocatore
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

// --- Chain interaction helpers ---
void Game::startChainWithActivation(const std::string& cardName, int ownerIdx){
    // Start a response window: push the initial activation and mark chain active
    effects.pushActivationRequest(cardName, ownerIdx);
    chainActive = true;
    lastResponder = -1;
    // Notify UI via event that a chain/response window started
    dispatcher.emit(GameEventType::SpellActivated);
}

bool Game::playerRespondWithActivation(int playerIdx, const std::string& cardName){
    if(!chainActive) return false;
    // Verify that the player can activate the card now (hand or set quick/trap). Caller must ensure correct cardName mapping.
    IActivationEffect* act = effects.getActivationFor(cardName);
    if(!act) return false;
    if(!act->canActivate(*this, playerIdx)) return false;
    // Push response on top
    effects.pushActivationRequest(cardName, playerIdx);
    lastResponder = playerIdx;
    // Signal UI/UI event
    dispatcher.emit(GameEventType::SpellActivated);
    return true;
}

void Game::playerPassOnChain(int playerIdx){
    if(!chainActive) return;
    // If both players passed (i.e., lastResponder != playerIdx and lastResponder != -1), resolve chain
    if(lastResponder != -1 && lastResponder != playerIdx){
        // both players passed consecutively -> resolve entire chain LIFO
        effects.resolveAllActivations(*this);
        chainActive = false;
        lastResponder = -1;
    } else {
        // mark that this player passed by setting lastResponder to playerIdx if no one has responded yet
        if(lastResponder == -1) lastResponder = playerIdx;
    }
}

void Game::registerEffectForCardName(const std::string& cardName, std::unique_ptr<ICardEffect> effect){
    effects.registerEffectForCardName(cardName, std::move(effect));
}

void Game::registerActivationForCardName(const std::string& cardName, std::unique_ptr<IActivationEffect> effect){
    effects.registerActivationForCardName(cardName, std::move(effect));
}

bool Game::tryNormalSet(size_t handIndex){
    Player& p = current();
    auto& hand = p.getHand();
    if(handIndex >= hand.size()) return false;
    if(normalSummonUsed) return false;
    if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
    int tributesNeeded = 0;
    if(auto lvl = hand[handIndex].getLevelOrRank(); lvl.has_value()){
        tributesNeeded = requiredTributesFor(hand[handIndex]);
    }
    if(tributesNeeded > 0){
        return beginNormalSummonWithTributes(handIndex, /*asSet*/true);
    }
    int cur = turn.getCurrentPlayerIndex();
    auto freeSlot = firstFreeMonsterSlot(cur);
    if(!freeSlot.has_value()) return false;
    return tryNormalSetToSlot(handIndex, *freeSlot);
}

bool Game::tryNormalSetToSlot(size_t handIndex, size_t slotIndex){
    Player& p = current();
    auto& hand = p.getHand();
    if(handIndex >= hand.size()) return false;
    if(normalSummonUsed) return false;
    if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
    int tributesNeeded = 0;
    if(auto lvl = hand[handIndex].getLevelOrRank(); lvl.has_value()){
        tributesNeeded = requiredTributesFor(hand[handIndex]);
    }
    int cur = turn.getCurrentPlayerIndex();
    if(tributesNeeded > 0){ pendingSummonSlotIndex = slotIndex; return beginNormalSummonWithTributes(handIndex, /*asSet*/true, pendingSummonSlotIndex); }
    if(slotIndex >= MONSTER_ZONE_SIZE) return false;
    if(isMonsterSlotOccupied(cur, slotIndex)) return false;
    if(!moveCard(CardZone::Hand, CardZone::MonsterZone, handIndex)) return false;
    if(!monsterIsDefense[cur].empty()) monsterIsDefense[cur].back() = true;
    if(!monsterIsFaceDown[cur].empty()) monsterIsFaceDown[cur].back() = true;
    if(!monsterSlotIndex[cur].empty()) monsterSlotIndex[cur].back() = static_cast<int>(slotIndex);
    normalSummonUsed = true;
    dispatcher.emit(GameEventType::NormalSet);
    return true;
}

// ================= Spells/Traps ================= //

bool Game::setSpellOrTrap(size_t handIndex){
    Player& p = current();
    auto& hand = p.getHand();
    if(handIndex >= hand.size()) return false;
    // Consentito solo in Main1/Main2
    if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
    // Deve essere Magia o Trappola (non FieldSpell)
    if(hand[handIndex].getType() != Type::SpellTrap) return false;
    if(spellTraps[turn.getCurrentPlayerIndex()].size() >= ST_ZONE_SIZE) return false;
    // Sposta nella Spell/Trap Zone coperta
    Card c = hand[handIndex];
    hand.erase(hand.begin()+handIndex);
    spellTraps[turn.getCurrentPlayerIndex()].push_back(std::move(c));
    int cur = turn.getCurrentPlayerIndex();
    stIsFaceDown[cur].push_back(true);
    stSetThisTurn[cur].push_back(true);
    // Evento differenziato SpellSet/TrapSet in base all'attributo (Magia/Trappola)
    if(c.getAttribute() == Attribute::Trappola){
        dispatcher.emit(GameEventType::TrapSet);
    } else {
        dispatcher.emit(GameEventType::SpellSet);
    }
    dispatcher.emit(GameEventType::CardMoved);
    return true;
}

bool Game::setFieldSpell(size_t handIndex){
    Player& p = current();
    auto& hand = p.getHand();
    if(handIndex >= hand.size()) return false;
    if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
    if(hand[handIndex].getType() != Type::FieldSpell) return false;
    int cur = turn.getCurrentPlayerIndex();
    // Rimpiazza eventuale Field esistente: manda quello vecchio al Cimitero
    if(fieldSpell[cur].has_value()){
        graveyard[cur].push_back(std::move(fieldSpell[cur].value()));
        lastSentToGrave = LastSentToGrave{ graveyard[cur].back().getName(), cur };
    std::cout << "[DEBUG] Game::setFieldSpell - lastSentToGrave name='" << lastSentToGrave->name << "' owner=" << lastSentToGrave->ownerIdx << " (setFieldSpell)" << std::endl;
        fieldSpell[cur].reset();
        dispatcher.emit(GameEventType::CardSentToGrave);
    }
    fieldSpell[cur] = hand[handIndex];
    hand.erase(hand.begin()+handIndex);
    dispatcher.emit(GameEventType::CardMoved);
    // Le Magie Terreno sono Magie Normali per attivazione (effetto immediato)
    dispatcher.emit(GameEventType::SpellActivated);
    return true;
}

bool Game::activateSpellFromHand(size_t handIndex){
    Player& p = current();
    auto& hand = p.getHand();
    if(handIndex >= hand.size()) return false;
    if(hand[handIndex].getType() != Type::SpellTrap && hand[handIndex].getType() != Type::FieldSpell) return false;
    // Fase: in linea base consenti in Main1/Main2 (per Rapide in futuro estendere a Battle/opp turn via chain)
    if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
    // Trappole non si attivano dalla mano
    if(hand[handIndex].getAttribute() == Attribute::Trappola) return false;
    // Magia Terreno: usa lo slot dedicato e resta sul campo
    if(hand[handIndex].getType() == Type::FieldSpell){
        return setFieldSpell(handIndex);
    }
    // Determina se Continua (resta sul campo) o Normale/Rituale/Rapida (va al Cimitero dopo la risoluzione)
    const auto &features = hand[handIndex].getFeatures();
    bool isContinuous = std::find(features.begin(), features.end(), Feature::Continua) != features.end();
    // Prima di estrarre dalla mano, interroga eventuale effetto di attivazione per i prerequisiti
    IActivationEffect* act = effects.getActivationFor(hand[handIndex].getName());
    int ownerIdx = turn.getCurrentPlayerIndex();
    if(act){
        if(!act->canActivate(*this, ownerIdx)){
            return false; // prerequisiti non soddisfatti
        }
    }
    // Estrai la carta dalla mano
    Card c = hand[handIndex];
    hand.erase(hand.begin()+handIndex);
    // Emissione evento di attivazione Magia
    dispatcher.emit(GameEventType::SpellActivated);
    if(isContinuous){
        int cur = turn.getCurrentPlayerIndex();
        // Inserisci nella zona M/T scoperta e attivabile subito
        if(spellTraps[cur].size() >= ST_ZONE_SIZE){
            // Nessuno spazio: rimetti in mano per non perdere la carta
            hand.insert(hand.begin()+handIndex, c);
            return false;
        }
        spellTraps[cur].push_back(std::move(c));
        stIsFaceDown[cur].push_back(false);
        stSetThisTurn[cur].push_back(false);
        dispatcher.emit(GameEventType::CardMoved);
    // Se l'oggetto continuo ha anche una risoluzione on-activate, pusha la richiesta
        if(act){
            // Avvia una response window: la UI può permettere ai giocatori di rispondere
            startChainWithActivation(hand[handIndex].getName(), ownerIdx);
        }
    } else {
        // Carta non-continua: apri una chain window se ha un effetto di attivazione,
        // poi sposta la carta al Cimitero subito (coerente con attivazioni da zona set).
        if(act){
            startChainWithActivation(c.getName(), ownerIdx);
        }
        int cur = turn.getCurrentPlayerIndex();
        graveyard[cur].push_back(std::move(c));
        lastSentToGrave = LastSentToGrave{ graveyard[cur].back().getName(), cur };
        std::cout << "[DEBUG] Game::activateSpellFromHand - lastSentToGrave name='" << lastSentToGrave->name << "' owner=" << lastSentToGrave->ownerIdx << " (activateSpellFromHand)" << std::endl;
        dispatcher.emit(GameEventType::CardSentToGrave);
        dispatcher.emit(GameEventType::CardMoved);
        return true;
    }
    return true;
}

bool Game::activateSetSpellTrap(size_t zoneIndex){
    int cur = turn.getCurrentPlayerIndex();
    // di default attiva per il giocatore di turno
    return activateSetSpellTrapFor(cur, zoneIndex);
}

bool Game::activateSetSpellTrapFor(int playerIdx, size_t zoneIndex){
    if(playerIdx < 0 || playerIdx > 1) return false;
    if(zoneIndex >= spellTraps[playerIdx].size()) return false;
    // Stato carta
    bool setThisTurn = (zoneIndex < stSetThisTurn[playerIdx].size() ? stSetThisTurn[playerIdx][zoneIndex] : false);
    Attribute attr = spellTraps[playerIdx][zoneIndex].getAttribute();
    const auto &features = spellTraps[playerIdx][zoneIndex].getFeatures();
    bool isQuick = std::find(features.begin(), features.end(), Feature::Rapida) != features.end();
    bool isContinuous = std::find(features.begin(), features.end(), Feature::Continua) != features.end();
    // Recupera eventuale effetto di attivazione registrato
    IActivationEffect* act = effects.getActivationFor(spellTraps[playerIdx][zoneIndex].getName());

    int cur = turn.getCurrentPlayerIndex();
    bool isOwnersTurn = (cur == playerIdx);

    // Timing rules (v0):
    // - Nel proprio turno: consenti attivazione solo in Main1/Main2
    // - Nel turno avversario: consenti SOLO Trappole o Magie Rapide, e non se settate in questo stesso turno
    if(isOwnersTurn){
        if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
        // Per il proprio turno, la restrizione "non nello stesso turno" si applica a Trappole/Rapide settate ora (come da regola base)
        if(setThisTurn && (attr == Attribute::Trappola || isQuick)) return false;
    } else {
        // Turno avversario
        if(!(attr == Attribute::Trappola || isQuick)) return false; // solo Trap/Quick
        if(setThisTurn) return false; // non nello stesso turno in cui è stata settata
        // Fasi: consentiamo in qualunque fase per ora (semplice response window v0)
    }

    // Se c'è un effetto registrato, verifica i prerequisiti prima di rivelare
    if(act){
        if(!act->canActivate(*this, playerIdx)) return false;
    }
    // Scopri (se era coperta)
    if(zoneIndex < stIsFaceDown[playerIdx].size()) stIsFaceDown[playerIdx][zoneIndex] = false;
    dispatcher.emit(attr == Attribute::Trappola ? GameEventType::TrapActivated : GameEventType::SpellActivated);

    // Risoluzione: se non-Continua manda al Cimitero, altrimenti resta
    if(!isContinuous){
        Card c = spellTraps[playerIdx][zoneIndex];
    // Esegui risoluzione tramite chain: pusha e risolvi LIFO
        if(act){
            startChainWithActivation(spellTraps[playerIdx][zoneIndex].getName(), playerIdx);
        }
        spellTraps[playerIdx].erase(spellTraps[playerIdx].begin()+zoneIndex);
        if(zoneIndex < stIsFaceDown[playerIdx].size()) stIsFaceDown[playerIdx].erase(stIsFaceDown[playerIdx].begin()+zoneIndex);
        if(zoneIndex < stSetThisTurn[playerIdx].size()) stSetThisTurn[playerIdx].erase(stSetThisTurn[playerIdx].begin()+zoneIndex);
        graveyard[playerIdx].push_back(std::move(c));
        lastSentToGrave = LastSentToGrave{ graveyard[playerIdx].back().getName(), playerIdx };
        std::cout << "[DEBUG] Game::destroyMonster - lastSentToGrave name='" << lastSentToGrave->name << "' owner=" << lastSentToGrave->ownerIdx << " (destroyMonster)" << std::endl;
        dispatcher.emit(GameEventType::CardSentToGrave);
    }
    else {
        // Continua: esegui comunque la risoluzione on-activate se prevista
        if(act){ act->resolve(*this, playerIdx); }
    }
    dispatcher.emit(GameEventType::CardMoved);
    return true;
}

bool Game::canActivateSetSpellTrapFor(int playerIdx, size_t zoneIndex){
    if(playerIdx < 0 || playerIdx > 1) return false;
    if(zoneIndex >= spellTraps[playerIdx].size()) return false;
    bool setThisTurn = (zoneIndex < stSetThisTurn[playerIdx].size() ? stSetThisTurn[playerIdx][zoneIndex] : false);
    Attribute attr = spellTraps[playerIdx][zoneIndex].getAttribute();
    const auto &features = spellTraps[playerIdx][zoneIndex].getFeatures();
    bool isQuick = std::find(features.begin(), features.end(), Feature::Rapida) != features.end();
    // Effetto registrato (se presente) deve poter essere attivato
    IActivationEffect* act = effects.getActivationFor(spellTraps[playerIdx][zoneIndex].getName());

    int cur = turn.getCurrentPlayerIndex();
    bool isOwnersTurn = (cur == playerIdx);

    if(isOwnersTurn){
        if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
        if(setThisTurn && (attr == Attribute::Trappola || isQuick)) return false;
    } else {
        if(!(attr == Attribute::Trappola || isQuick)) return false;
        if(setThisTurn) return false;
        // Fasi: per ora consentito in qualunque fase del turno avversario
    }
    if(act){
        if(!act->canActivate(*this, playerIdx)) return false;
    }
    return true;
}

bool Game::isSpellTrapFaceDownAt(int playerIdx, size_t zoneIndex) const{
    if(playerIdx < 0 || playerIdx > 1) return false;
    if(zoneIndex >= stIsFaceDown[playerIdx].size()) return false;
    return stIsFaceDown[playerIdx][zoneIndex];
}

bool Game::isSpellTrapSetThisTurnAt(int playerIdx, size_t zoneIndex) const{
    if(playerIdx < 0 || playerIdx > 1) return false;
    if(zoneIndex >= stSetThisTurn[playerIdx].size()) return false;
    return stSetThisTurn[playerIdx][zoneIndex];
}

bool Game::specialSummonToMonsterZone(int playerIdx, Card&& c, bool defense, bool faceDown){
    if(playerIdx < 0 || playerIdx > 1) return false;
    if(monsterZones[playerIdx].size() >= MONSTER_ZONE_SIZE) return false;
    monsterZones[playerIdx].push_back(std::move(c));
    // Assicura che i vettori di stato siano allineati
    if(monsterHasAttacked[playerIdx].size() < monsterZones[playerIdx].size())
        monsterHasAttacked[playerIdx].push_back(false);
    if(monsterIsDefense[playerIdx].size() < monsterZones[playerIdx].size())
        monsterIsDefense[playerIdx].push_back(defense);
    if(monsterIsFaceDown[playerIdx].size() < monsterZones[playerIdx].size())
        monsterIsFaceDown[playerIdx].push_back(faceDown);
    if(monsterSummonedThisTurn[playerIdx].size() < monsterZones[playerIdx].size())
        monsterSummonedThisTurn[playerIdx].push_back(false); // evocato da effetto, non conta come Normal Summon
    if(monsterPositionChangedThisTurn[playerIdx].size() < monsterZones[playerIdx].size())
        monsterPositionChangedThisTurn[playerIdx].push_back(false);
    // assegna primo slot libero disponibile
    auto freeSlot = firstFreeMonsterSlot(playerIdx);
    monsterSlotIndex[playerIdx].push_back(freeSlot.has_value()? static_cast<int>(*freeSlot) : -1);
    dispatcher.emit(GameEventType::CardMoved);
    return true;
}

bool Game::requestSpecialSummonWithChoice(int playerIdx, Card&& c){
    if(playerIdx < 0 || playerIdx > 1) return false;
    if(monsterZones[playerIdx].size() >= MONSTER_ZONE_SIZE) return false;
    if(pendingSS.has_value()) return false; // già in corso
    pendingSS = PendingSS{ playerIdx, std::move(c) };
    // Notifica la UI che deve chiedere Attacco o Difesa (coperto non permesso)
    dispatcher.emit(GameEventType::SpecialSummonChoiceRequested);
    return true;
}

std::optional<int> Game::getPendingSpecialSummonOwner() const{
    if(!pendingSS.has_value()) return std::nullopt;
    return pendingSS->ownerIdx;
}

bool Game::resolvePendingSpecialSummon(bool choiceDefense){
    if(!pendingSS.has_value()) return false;
    int owner = pendingSS->ownerIdx;
    Card card = std::move(pendingSS->card);
    pendingSS.reset();
    // Evoca in campo con la posizione scelta, sempre scoperto (faceDown=false)
    return specialSummonToMonsterZone(owner, std::move(card), /*defense*/choiceDefense, /*faceDown*/false);
}


std::optional<int> Game::getPendingSendOwner() const{
    if(!pendingSend.has_value()) return std::nullopt;
    return pendingSend->ownerIdx;
}

bool Game::resolvePendingSendFromDeck(size_t indexInFiltered){
    if(!pendingSend.has_value()) return false;
    int owner = pendingSend->ownerIdx;
    Deck& deck = getDeckOf(owner);
    // Ricava la lista filtrata (Draghi) nello stesso ordine della UI
    auto candidates = deck.collectWhere([](const Card& c){
        const auto &feats = c.getFeatures();
        return std::find(feats.begin(), feats.end(), Feature::Drago) != feats.end();
    });
    if(indexInFiltered >= candidates.size()) return false;
    std::string name = candidates[indexInFiltered].getName();
    auto removed = deck.removeFirstByName(name);
    if(!removed.has_value()) return false;
    // Mantieni coerente il Deck esterno usato dall'UI; passa il deck sorgente per evitare double-remove
    mirrorExternalDeckRemoveByName(name, &deck);
    // Inserisci nel Cimitero corretto e notifica
    graveyard[owner].push_back(std::move(removed.value()));
    lastSentToGrave = LastSentToGrave{ graveyard[owner].back().getName(), owner };
    dispatcher.emit(GameEventType::CardSentToGrave);
    dispatcher.emit(GameEventType::CardMoved);
    pendingSend.reset();
    return true;
}

void Game::cancelPendingSendFromDeck(){
    pendingSend.reset();
}

bool Game::setPosition(size_t zoneIndex, bool defense, bool faceDown, bool allowByEffect){
    int cur = turn.getCurrentPlayerIndex();
    if(zoneIndex >= monsterZones[cur].size()) return false;
    if(zoneIndex >= monsterIsDefense[cur].size() || zoneIndex >= monsterIsFaceDown[cur].size()) return false;
    // Consentito solo in Main1/Main2, salvo effetti
    if(!allowByEffect){
        if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
    }
    // Regola: se il mostro è stato evocato in questo turno, non può cambiare posizione
    if(zoneIndex < monsterSummonedThisTurn[cur].size() && monsterSummonedThisTurn[cur][zoneIndex]){
        return false;
    }
    // Regola: può cambiare posizione solo una volta per turno
    if(zoneIndex < monsterPositionChangedThisTurn[cur].size() && monsterPositionChangedThisTurn[cur][zoneIndex]){
        return false;
    }
    // Nuove regole direzionali:
    // - Se coperto (faceDown==true nello stato attuale), può solo andare in attacco scoperto
    // - Se in attacco (defense==false nello stato attuale e faceDown==false), può solo andare in difesa
    // - Se in difesa (defense==true nello stato attuale), può solo andare in attacco
    bool curDef = monsterIsDefense[cur][zoneIndex];
    bool curFD  = monsterIsFaceDown[cur][zoneIndex];
    if(curFD){
        // Deve diventare attacco scoperto
        if(!(defense == false && faceDown == false)) return false;
    } else if(!curDef){
        // Attacco scoperto -> solo difesa scoperta
        if(!(defense == true && faceDown == false)) return false;
    } else {
        // Difesa (coperta o scoperta già escluso da curFD): solo attacco scoperto
        if(!(defense == false && faceDown == false)) return false;
    }
    bool wasFaceDown = monsterIsFaceDown[cur][zoneIndex];
    monsterIsDefense[cur][zoneIndex] = defense;
    monsterIsFaceDown[cur][zoneIndex] = faceDown;
    if(zoneIndex < monsterPositionChangedThisTurn[cur].size()){
        monsterPositionChangedThisTurn[cur][zoneIndex] = true;
    }
    dispatcher.emit(GameEventType::CardMoved);
    if(wasFaceDown && !faceDown){
        dispatcher.emit(GameEventType::MonsterFlipped);
    }
    return true;
}

bool Game::togglePosition(size_t zoneIndex, bool allowByEffect){
    int cur = turn.getCurrentPlayerIndex();
    if(zoneIndex >= monsterZones[cur].size()) return false;
    if(zoneIndex >= monsterIsDefense[cur].size() || zoneIndex >= monsterIsFaceDown[cur].size()) return false;
    // Stessa regola di fase (salvo effetti)
    if(!allowByEffect){
        if(turn.getPhase() != GamePhase::Main1 && turn.getPhase() != GamePhase::Main2) return false;
    }
    bool curDef = monsterIsDefense[cur][zoneIndex];
    bool curFD  = monsterIsFaceDown[cur][zoneIndex];
    if(curFD){
        // Da coperto -> attacco scoperto
        return setPosition(zoneIndex, /*defense*/false, /*faceDown*/false, allowByEffect);
    }
    if(!curDef){
        // Attacco scoperto -> difesa scoperta
        return setPosition(zoneIndex, /*defense*/true, /*faceDown*/false, allowByEffect);
    }
    // Difesa -> attacco scoperto
    return setPosition(zoneIndex, /*defense*/false, /*faceDown*/false, allowByEffect);
}

bool Game::isDefenseAt(int playerIdx, size_t zoneIndex) const{
    if(playerIdx < 0 || playerIdx > 1) return false;
    if(zoneIndex >= monsterIsDefense[playerIdx].size()) return false;
    return monsterIsDefense[playerIdx][zoneIndex];
}

bool Game::isFaceDownAt(int playerIdx, size_t zoneIndex) const{
    if(playerIdx < 0 || playerIdx > 1) return false;
    if(zoneIndex >= monsterIsFaceDown[playerIdx].size()) return false;
    return monsterIsFaceDown[playerIdx][zoneIndex];
}

int Game::getMonsterSlotIndexAt(int playerIdx, size_t zoneIndex) const{
    if(playerIdx < 0 || playerIdx > 1) return -1;
    if(zoneIndex >= monsterSlotIndex[playerIdx].size()) return -1;
    return monsterSlotIndex[playerIdx][zoneIndex];
}

bool Game::isMonsterSlotOccupied(int playerIdx, size_t slotIndex) const{
    if(playerIdx < 0 || playerIdx > 1) return true;
    for(size_t i=0;i<monsterSlotIndex[playerIdx].size();++i){
        if(monsterSlotIndex[playerIdx][i] == static_cast<int>(slotIndex)) return true;
    }
    return false;
}

std::optional<size_t> Game::firstFreeMonsterSlot(int playerIdx) const{
    for(size_t s=0;s<MONSTER_ZONE_SIZE;++s){
        if(!isMonsterSlotOccupied(playerIdx, s)) return s;
    }
    return std::nullopt;
}
