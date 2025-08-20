#pragma once
#include "Player.h"
#include "TurnManager.h"
#include <array>

class Game {
    public:

        Game(const Player& p1, const Player& p2): players{p1,p2} {}

        void start(){
            if(started) return;
            // pesca iniziale 5 carte ciascuno
            for(int i=0;i<5;++i){
                players[0].drawOne();
                players[1].drawOne();
            }
            started = true;
        }

        Player& current(){ return players[turn.currentPlayer]; }
        Player& opponent(){ return players[1 - turn.currentPlayer]; }
        
    private:
        std::array<Player,2> players;
        TurnManager turn;
        bool started = false;
};
