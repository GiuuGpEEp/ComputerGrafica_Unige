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
    NormalSet,
    SpellSet,
    TrapSet,
    SpellActivated,
    TrapActivated,
    MonsterFlipped, // nuovo: un mostro viene scoperto (Flip) sul campo
    TurnStart,
    TurnEnd,
    StandbySkip,
    CardMoved,
    CardSentToGrave,
    MonstersTributed,
    NormalSummonTributeRequired,
    // Richiesta scelta posizione per Evocazione Speciale (Attacco o Difesa). Nessuna copertura consentita
    SpecialSummonChoiceRequested,
    // Richiesta scelta carta dal Deck da mandare al Cimitero (es. Reliquario del Drago)
    DeckSendChoiceRequested,
    // Richiesta scelta carta dal Deck da aggiungere alla mano (es. Melodia del Drago)
    DeckAddChoiceRequested,
    // Richiesta scelta carta dalla mano da scartare (es. costo Melodia del Drago)
    HandDiscardChoiceRequested,
    // Richiesta scelta bersaglio dal Cimitero (es. Richiamo del Posseduto)
    GraveyardChoiceRequested,
    // Richiesta scelta di un proprio mostro sul terreno (per effetti come Saggio con Occhi di Blu)
    OwnMonsterChoiceRequested,
    // Richiesta: aggiungi dalla mano un Drago di Livello 1 dal Deck (saggio on-summon)
    DeckAddLevel1DragonChoiceRequested,
    // Richiesta: dopo il sacrificio del Saggio, scegli un "Drago Bianco Occhi Blu" o "Drago Spirito" da mano/Deck/Cimitero per l'Evocazione Speciale
    BlueEyesSSChoiceRequested,
    // Richiesta: seleziona una Magia/Trappola controllata dall'avversario (inclusa eventuale Magia Terreno) da bandire
    OpponentSpellTrapChoiceRequested,
    BattleStart,
    BattleEnd,
    AttackDeclared,
    AttackResolved,
    // Emesso quando un mostro viene Evocato Specialmente sul campo (post CardMoved)
    MonsterSpecialSummoned,
    MonsterDestroyed,
    LifePointsChanged,
    DirectAttack,
    Win,
    Lose
};

class EventDispatcher {
public:
    using Handler = std::function<void()>;

    int subscribe(GameEventType type, Handler cb){
        int id = ++lastId; listeners[type].push_back({id, std::move(cb)}); return id; }

    // Annulla la sottoscrizione di un listener tramite id. Se id==0 non fa nulla.
    void unsubscribe(int id){ if(id==0) return; for(auto &kv : listeners){ auto &vec = kv.second; vec.erase(std::remove_if(vec.begin(), vec.end(), [id](const Listener &l){ return l.id==id; }), vec.end()); } }

    void emit(GameEventType type){
        auto it = listeners.find(type); if(it==listeners.end()) return; for(auto &pair : it->second){ pair.cb(); }
    }

    void clear(){ listeners.clear(); }
private:
    struct Listener { int id; Handler cb; };
    std::unordered_map<GameEventType, std::vector<Listener>> listeners; int lastId = 0;
};
