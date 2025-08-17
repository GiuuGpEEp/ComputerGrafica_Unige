#pragma once
#include "../resources/jsonData/json.hpp"
#include "../Deck/Deck.h"
#include "../TextureManager/TextureManager.h"
#include <unordered_map>
#include <vector>
#include <string>

class ResourceManager {
private:
    TextureManager& textureManager;
    std::unordered_map<std::string, Card> allCards;
    std::vector<Deck> availableDecks;
    std::string resourceBasePath;
    
    void loadCardsFromJson(const std::string& cardsJsonPath, const sf::Vector2f& deckSlotPos, const sf::Vector2f& deckCardSize);
    void loadDecksFromJson(const std::string& decksJsonPath, const sf::Vector2f& deckSlotPos, const sf::Vector2f& deckCardSize, const sf::Vector2f& slotSize, const sf::Texture& textureFlipped);

public:
    ResourceManager(TextureManager& tm, const std::string& resourceBase);
    
    bool initialize(const sf::Vector2f& deckSlotPos, const sf::Vector2f& deckCardSize, const sf::Vector2f& slotSize, const sf::Texture& textureFlipped);
    
    const std::vector<Deck>& getAvailableDecks() const { return availableDecks; }
    const std::unordered_map<std::string, Card>& getAllCards() const { return allCards; }
    
    Deck getDeckByName(const std::string& deckName) const;
    Card getCardByName(const std::string& cardName) const;
};