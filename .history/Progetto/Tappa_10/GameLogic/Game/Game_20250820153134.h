#pragma once
//#include "../Player/Player.h"
#include "../TurnManager/TurnManager.h"
#include <array>

class Game {
public:
    Game(const Player& p1, const Player& p2);
    void start();
    bool isStarted() const { return started; }

    Player& current();
    Player& opponent();
    const TurnManager& getTurn() const { return turn; }
    TurnManager& getTurn() { return turn; }

private:
    std::array<Player,2> players;
    TurnManager turn{0, GamePhase::Draw};
    bool started = false;
};
