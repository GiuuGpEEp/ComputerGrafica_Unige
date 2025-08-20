#pragma once
#include "../Player/Player.h"
#include "../TurnManager/TurnManager.h"
#include <array>

class Game {
public:
    Game(const Player& p1, const Player& p2);
    void start();
    bool isStarted() const;

    Player& current();
    Player& opponent();
    TurnManager& getTurn();

private:
    std::array<Player,2> players;
    TurnManager turn{0, GamePhase::Draw};
    bool started = false;
};
