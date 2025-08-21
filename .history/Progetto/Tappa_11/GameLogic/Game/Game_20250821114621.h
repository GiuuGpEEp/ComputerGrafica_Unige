#pragma once
#include "../Player/Player.h"
#include "../TurnManager/TurnManager.h"
#include <array>
#include <optional>
#include "../Events/EventDispatcher.h"

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

    // Normal Summon tracking
    bool canNormalSummon() const;
    bool tryNormalSummon(size_t handIndex); // per ora solo aggiunge a zona mostri logica
    void resetNormalSummon();
    const std::vector<Card>& getMonsterZone() const; // accesso read-only
    std::vector<Card>& getGraveyard();
    void discardExcess(size_t handLimit);

private:
    std::array<Player,2> players;
    TurnManager turn{0, GamePhase::Draw};
    bool started = false;
    bool normalSummonUsed = false;
    static constexpr size_t MONSTER_ZONE_SIZE = 3;
    std::array<std::vector<Card>,2> monsterZones; // una per player
    std::vector<Card> graveyard;
    EventDispatcher dispatcher;
};
