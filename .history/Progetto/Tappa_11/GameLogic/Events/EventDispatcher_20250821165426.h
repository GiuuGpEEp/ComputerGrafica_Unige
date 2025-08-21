#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <cstdint>

enum class GameEventType{
    PhaseChange,
    DrawStart,
    DrawEnd,
    NormalSummon,
    TurnStart,
    TurnEnd,
    StandbySkip,
    CardMoved,
    CardSentToGrave,
    MonstersTributed
};

class EventDispatcher {
public:
    using Handler = std::function<void()>;

    int subscribe(GameEventType type, Handler cb){
        int id = ++lastId; listeners[type].push_back({id, std::move(cb)}); return id; }

    void emit(GameEventType type){
        auto it = listeners.find(type); if(it==listeners.end()) return; for(auto &pair : it->second){ pair.cb(); }
    }

    void clear(){ listeners.clear(); }
private:
    struct Listener { int id; Handler cb; };
    std::unordered_map<GameEventType, std::vector<Listener>> listeners; int lastId = 0;
};
