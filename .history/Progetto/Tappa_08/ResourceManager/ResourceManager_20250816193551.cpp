#include "ResourceManager.h"
#include <fstream>
#include <iostream>

ResourceManager::ResourceManager(TextureManager& tm, const std::string& resourceBase) 
    : textureManager(tm), resourceBasePath(resourceBase) {}

bool ResourceManager::initialize(const sf::Vector2f& deckSlotPos, const sf::Vector2f& deckCardSize, const sf::Vector2f& slotSize, const sf::Texture& textureFlipped) {
    try {
        std::string cardsJsonPath = resourceBasePath + "jsonData/cards.json";
        std::string decksJsonPath = resourceBasePath + "jsonData/decks.json";
        
        loadCardsFromJson(cardsJsonPath, deckSlotPos, deckCardSize);
        loadDecksFromJson(decksJsonPath, deckSlotPos, deckCardSize, slotSize, textureFlipped);
        
        std::cout << "ResourceManager inizializzato con successo!" << std::endl;
        std::cout << "Carte caricate: " << allCards.size() << std::endl;
        std::cout << "Deck disponibili: " << availableDecks.size() << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "ERRORE nell'inizializzazione del ResourceManager: " << e.what() << std::endl;
        return false;
    }
}

void ResourceManager::loadCardsFromJson(const std::string& cardsJsonPath, const sf::Vector2f& deckSlotPos, const sf::Vector2f& deckCardSize) {
    std::ifstream cardsFile(cardsJsonPath);
    if (!cardsFile.is_open()) {
        throw std::runtime_error("Impossibile aprire il file JSON delle carte: " + cardsJsonPath);
    }
    
    nlohmann::json cardsJson;
    cardsFile >> cardsJson;
    
    for (const auto& cardJson : cardsJson) {
        try {
            Card card = Card::cardFromJson(cardJson, deckSlotPos, deckCardSize, textureManager);
            allCards[card.getName()] = card;
        } catch (const std::exception& e) {
            std::cerr << "ERRORE nella creazione della carta: "
                      << (cardJson.contains("name") ? cardJson["name"].get<std::string>() : "<senza nome>")
                      << " - " << e.what() << std::endl;
        }
    }
}

void ResourceManager::loadDecksFromJson(const std::string& decksJsonPath, const sf::Vector2f& deckSlotPos, const sf::Vector2f& deckCardSize, const sf::Vector2f& slotSize, const sf::Texture& textureFlipped) {
    std::ifstream deckFile(decksJsonPath);
    if (!deckFile.is_open()) {
        throw std::runtime_error("Impossibile aprire il file JSON dei deck: " + decksJsonPath);
    }
    
    nlohmann::json decksJson;
    deckFile >> decksJson;
    
    // Assumendo che il JSON contenga un array di deck
    if (decksJson.is_array()) {
        for (const auto& deckJson : decksJson) {
            try {
                Deck deck = Deck::deckFromJson(deckJson, allCards, deckSlotPos, deckCardSize, slotSize, textureFlipped);
                availableDecks.push_back(deck);
            } catch (const std::exception& e) {
                std::cerr << "ERRORE nella creazione del deck: " << e.what() << std::endl;
            }
        }
    } else {
        // Gestione del caso in cui il JSON contenga un singolo deck
        Deck deck = Deck::deckFromJson(decksJson, allCards, deckSlotPos, deckCardSize, slotSize, textureFlipped);
        availableDecks.push_back(deck);
    }
}

Deck ResourceManager::getDeckByName(const std::string& deckName) const {
    for (const auto& deck : availableDecks) {
        if (deck.getDeckName() == deckName) {
            return deck;
        }
    }
    throw std::runtime_error("Deck non trovato: " + deckName);
}

Card ResourceManager::getCardByName(const std::string& cardName) const {
    auto it = allCards.find(cardName);
    if (it != allCards.end()) {
        return it->second;
    }
    throw std::runtime_error("Carta non trovata: " + cardName);
}