#pragma once
#include <vector>
#include <string>
#include <optional>
#include "../../Deck/Deck.h"

class Player {
public:
    // Ora memorizziamo un puntatore non-owning a un Deck fornito dal chiamante.
    // Questo evita copie accidentali del Deck e mantiene sincronizzati UI e logica di gioco.
    Player(const std::string& n, Deck& d);
    bool canDraw() const;
    std::optional<Card> drawOne();

    const std::string& getName() const;
    int getLifePoints() const;
    void setLifePoints(int lp); // set assoluto
    void damage(int amount);    // sottrae LP
    std::vector<Card>& getHand();
    const Deck& getDeck() const;
    Deck& getDeck();
    void clearHand();

private:
    std::string name;
    Deck* deckPtr; // non-owning pointer to shared Deck instance
    std::vector<Card> hand;
    std::vector<Card> fieldMonsters; // placeholder
    std::vector<Card> fieldSpellTraps; // placeholder
    int lifePoints = 8000; 
};
