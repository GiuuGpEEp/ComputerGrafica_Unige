#pragma once
#include "Player.h"
#include "TurnManager.h"
#include <array>

class Game {
    public:

        Game(const Player& p1, const Player& p2);

        void start()

        Player& current(){ return players[turn.currentPlayer]; }
        Player& opponent(){ return players[1 - turn.currentPlayer]; }

    private:
        std::array<Player,2> players;
        TurnManager turn;
        bool started = false;
};
