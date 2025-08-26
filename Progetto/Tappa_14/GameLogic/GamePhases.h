#pragma once
#include <string>

enum class GamePhase {
    Draw,
    Standby,
    Main1,
    Battle,
    Main2,
    End
};

inline std::string phaseToString(GamePhase p){
    switch(p){
        case GamePhase::Draw: return "Draw";
        case GamePhase::Standby: return "Standby";
        case GamePhase::Main1: return "Main1";
        case GamePhase::Battle: return "Battle";
        case GamePhase::Main2: return "Main2";
        case GamePhase::End: return "End";
    }
    return "Unknown";
}
