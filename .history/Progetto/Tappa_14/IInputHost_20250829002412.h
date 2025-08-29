#pragma once

#include <optional>
#include <functional>
#include <SFML/System.hpp>

class Game; // forward
class Card;

struct IInputHost {
    virtual ~IInputHost() = default;
    virtual void restoreCardAtHandIndex(size_t idx) = 0;
    virtual void updateHandLayout() = 0;
    virtual void syncMonsterZoneToField() = 0;
    virtual void syncSpellTrapZones() = 0;
    virtual Game* getGame() = 0;
    virtual void confirmDeckSend(size_t idx) = 0;
    virtual void onPlayClicked() = 0;
    virtual void onStartDeckSelection() = 0;
    virtual void onSelectDeckIndex(int idx) = 0;
    virtual std::optional<int> findMonsterSlotIndexUnderMouse(sf::Vector2f mp) = 0;
};
