#pragma once
#include <optional>

class Game;

class AIController {
public:
    explicit AIController(int aiPlayerIdx = 1) : aiPlayerIdx_(aiPlayerIdx) {}
    void attachGame(Game* g) { game_ = g; resetPhaseState(); }
    void update(float dt);

private:
    Game* game_ = nullptr;
    int aiPlayerIdx_ = 1; // Player 2 (alto)

    // Stato per non ripetere azioni ogni frame
    int lastTurnCount_ = -1;
    int lastPhase_ = -1;
    bool actedInThisPhase_ = false;

    void onNewPhase();
    void doMain1();
    void doBattle();
    void doMain2();

    // Helpers
    std::optional<size_t> findFirstFreeMonsterSlot() const;
    std::optional<size_t> pickSummonFromHand() const;   // Mostro Lvl <= 4
    std::optional<size_t> pickNormalSpellFromHand() const; // Magia attivabile dalla mano (Game gaterà Trappole)
    void tryActivateNormalSpells();
    void trySetTrapsAndQuick();
    std::optional<size_t> pickBestAttackTarget() const; // indice zona avversaria
    void resetPhaseState();
};
