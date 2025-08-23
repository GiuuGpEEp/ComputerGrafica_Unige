#pragma once
#include "../Player/Player.h"
#include "../TurnManager/TurnManager.h"
#include <array>
#include <optional>
#include "../Events/EventDispatcher.h"
#include <functional>

// Forward declarations per evitare dipendenze circolari
class DrawController;

enum class CardZone{ 
    Deck, 
    Hand, 
    MonsterZone, 
    Graveyard 
};

class Game {
public:
    Game(const Player& p1, const Player& p2);
    void start();
    bool isStarted() const;

    Player& current();
    Player& opponent();
    TurnManager& getTurn();
    EventDispatcher& events();
    void advancePhase();
    void endTurn();
    void startTurn(); // nuovo inizio turno centralizzato
    int getStartingPlayerIndex() const { return startingPlayerIndex; }
    bool isGameOver() const { return gameOver; }
    std::optional<int> getWinnerIndex() const { return winnerIndex; }

    // Collegamento controller animazioni pescata (gestione automatica pescate Draw Phase + mano iniziale)
    void attachDrawController(DrawController* controller);

    // Gestione limite mano / scarto
    void setHandLimit(size_t limit); // imposta limite per auto discard
    void setDiscardCallback(std::function<void(std::vector<Card>&&)> cb); // callback per avviare animazione scarto nel layer rendering
    void fastForwardToEndPhase(); // avanza direttamente a End Phase applicando regole
    void onDiscardAnimationFinished(); // chiamato dal main quando animazione scarti termina
    bool shouldAutoEndTurn() const; // true se End Phase senza scarti e turno da chiudere

    // Movimento generico carte (per ora solo del giocatore corrente)
    // Ritorna true se spostamento riuscito
    bool moveCard(CardZone from, CardZone to, size_t indexFrom);
    // Tribute logic
    int requiredTributesFor(const Card& c) const; // 0/1/2 in base al livello
    // Indici dei mostri da tributare (rispetto alla monster zone corrente). Ritorna true se ok.
    bool tributeMonsters(const std::vector<size_t>& zoneIndices);
    // Avvia processo di Normal Summon che richiede tributi: salva indice carta in mano e attende scelta
    bool beginNormalSummonWithTributes(size_t handIndex);
    // Completa la summon dopo che l'UI ha fornito gli indici dei tributi; ritorna true se successo
    bool completePendingNormalSummon(const std::vector<size_t>& tributeIndices);
    bool hasPendingNormalSummon() const { return pendingSummonHandIndex.has_value(); }
    void cancelPendingNormalSummon();
    int getPendingTributesNeeded() const { return pendingSummonHandIndex.has_value() ? pendingTributesNeeded : 0; }

    // Normal Summon tracking
    bool canNormalSummon() const;
    bool tryNormalSummon(size_t handIndex); // per ora solo aggiunge a zona mostri logica
    void resetNormalSummon();
    const std::vector<Card>& getMonsterZone() const; // accesso read-only
    const std::vector<Card>& getOpponentMonsterZone() const; // zona mostri dell'avversario (read-only)
    std::vector<Card>& getGraveyard();
    void discardExcess(size_t handLimit);
    // Estrae le carte oltre il limite (le rimuove dalla mano ma non le mette ancora nel graveyard)
    std::vector<Card> extractExcessCards(size_t handLimit);

    // Helper per End Phase: estrae carte in eccesso dalla mano (per animazione scarto esterna)
    std::vector<Card> handleEndPhase(size_t handLimit);

    // Battle API (v1)
    bool canDeclareAttack(size_t attackerIndex, std::optional<size_t> targetIndex) const;
    bool declareAttack(size_t attackerIndex, std::optional<size_t> targetIndex);

    // Debug helpers
    void debugAddMonsterToOpponent(const Card& c);

private:
    void checkVictoryByLP();
    void handleDeckOutIfAny();
    void processEnterEndPhase();
    void processEnterStandby();
    std::array<Player,2> players;
    TurnManager turn{0, GamePhase::Draw};
    int startingPlayerIndex = 0; // 0 = Player 1
    bool started = false;
    bool firstTurn = true; // per saltare pescata iniziale giocatore che inizia
    bool normalSummonUsed = false;
    static constexpr size_t MONSTER_ZONE_SIZE = 3;
    std::array<std::vector<Card>,2> monsterZones; // una per player
    std::vector<Card> graveyard;
    EventDispatcher dispatcher;
    DrawController* drawCtrl = nullptr; // non-owning
    size_t handLimit = 7; // default
    std::function<void(std::vector<Card>&&)> discardCallback; // avvio animazione scarti
    bool pendingAutoTurnEnd = false; // End Phase senza scarti -> chiudere turno dopo un frame
    // Stato per Normal Summon con tributi
    std::optional<size_t> pendingSummonHandIndex; // indice carta in mano da evocare dopo scelta tributi
    int pendingTributesNeeded = 0;

    // Battle state: flag "ha attaccato" per ciascun mostro in zona, per giocatore
    std::array<std::vector<bool>,2> monsterHasAttacked;

    // Victory state
    bool gameOver = false;
    std::optional<int> winnerIndex; // 0/1

    // Helpers battle
    void destroyMonster(int playerIdx, size_t zoneIndex);
    void dealDamageTo(int playerIdx, int amount);
};
