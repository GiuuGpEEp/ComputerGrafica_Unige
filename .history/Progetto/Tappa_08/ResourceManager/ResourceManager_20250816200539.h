#pragma once
#include "../resources/jsonData/json.hpp"
#include "../Deck/Deck.h"
#include "../Card/Card.h"
#include "../TextureManager/TextureManager.h"
#include <unordered_map>
#include <vector>
#include <string>

class ResourceManager {

    public:
        ResourceManager(TextureManager& tm, const std::string& resourceBase);

        bool initialize(const sf::Vector2f& deckSlotPos, const sf::Vector2f& deckCardSize, const sf::Vector2f& slotSize, sf::Texture& textureFlipped);

        // Metodi per accedere ai deck
        const std::unordered_map<std::string, Deck>& getAvailableDecks() const;
        const std::unordered_map<std::string, Card>& getAllCards() const;

        // Accesso diretto per nome (più efficiente)
        const Deck* getDeckByName(const std::string& deckName) const;
        const Card* getCardByName(const std::string& cardName) const;

        // Metodi helper
        std::vector<std::string> getDeckNames() const;
        size_t getDeckCount() const { return availableDecks.size(); }
        bool hasDeck(const std::string& deckName) const
        bool hasCard(const std::string& cardName) const { return allCards.find(cardName) != allCards.end(); }

    private:
        TextureManager& textureManager;
        std::unordered_map<std::string, Card> allCards;
        std::unordered_map<std::string, Deck> availableDecks; // Cambiato da vector a unordered_map
        std::string resourceBasePath;
        
        void loadCardsFromJson(const std::string& cardsJsonPath, const sf::Vector2f& deckSlotPos, const sf::Vector2f& deckCardSize);
        void loadDecksFromJson(const std::string& decksJsonPath, const sf::Vector2f& deckSlotPos, const sf::Vector2f& deckCardSize, const sf::Vector2f& slotSize, sf::Texture& textureFlipped);
    
};