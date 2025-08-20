#pragma once
#include "../Player/Player.h"
#include "../TurnManager/TurnManager.h"
#include <array>
#include <optional>

class Game {
public:
    Game(const Player& p1, const Player& p2);
    void start();
    bool isStarted() const;

    Player& current();
    Player& opponent();
    TurnManager& getTurn();

    // Normal Summon tracking
    bool canNormalSummon() const;
    bool tryNormalSummon(size_t handIndex); // per ora solo aggiunge a zona mostri logica
    void resetNormalSummon();
    const std::vector<Card>& getMonsterZone() const; // accesso read-only

private:
    std::array<Player,2> players;
    TurnManager turn{0, GamePhase::Draw};
    bool started = false;
    bool normalSummonUsed = false;
    static constexpr size_t MONSTER_ZONE_SIZE = 5;
    std::vector<Card> monsterZone; // semplice rappresentazione (solo player corrente per ora)
};
