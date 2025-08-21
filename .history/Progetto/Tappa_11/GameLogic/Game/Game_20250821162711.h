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
    Deck, Hand, MonsterZone, Graveyard };

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

    // Normal Summon tracking
    bool canNormalSummon() const;
    bool tryNormalSummon(size_t handIndex); // per ora solo aggiunge a zona mostri logica
    void resetNormalSummon();
    const std::vector<Card>& getMonsterZone() const; // accesso read-only
    std::vector<Card>& getGraveyard();
    void discardExcess(size_t handLimit);
    // Estrae le carte oltre il limite (le rimuove dalla mano ma non le mette ancora nel graveyard)
    std::vector<Card> extractExcessCards(size_t handLimit);

    // Helper per End Phase: estrae carte in eccesso dalla mano (per animazione scarto esterna)
    std::vector<Card> handleEndPhase(size_t handLimit);

private:
    void processEnterEndPhase();
    void processEnterStandby();
    std::array<Player,2> players;
    TurnManager turn{0, GamePhase::Draw};
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
};
