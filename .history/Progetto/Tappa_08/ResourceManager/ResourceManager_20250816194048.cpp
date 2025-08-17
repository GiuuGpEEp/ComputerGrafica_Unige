#include "ResourceManager.h"
#include <fstream>
#include <iostream>

ResourceManager::ResourceManager(TextureManager& tm, const std::string& resourceBase) 
    : textureManager(tm), resourceBasePath(resourceBase) {}

bool ResourceManager::initialize(const sf::Vector2f& deckSlotPos, const sf::Vector2f& deckCardSize, const sf::Vector2f& slotSize, const sf::Texture& textureFlipped) {
    try {
        std::string cardsJsonPath = resourceBasePath + "jsonData/cards.json";
        std::string decksJsonPath = resourceBasePath + "jsonData/decks.json";
        
        std::cout << "Tentativo di caricamento carte da: " << cardsJsonPath << std::endl;
        std::cout << "Tentativo di caricamento deck da: " << decksJsonPath << std::endl;
        
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
    try {
        cardsFile >> cardsJson;
    } catch (const std::exception& e) {
        throw std::runtime_error("Errore nel parsing del JSON delle carte: " + std::string(e.what()));
    }
    
    if (!cardsJson.is_array()) {
        throw std::runtime_error("Il JSON delle carte deve essere un array");
    }
    
    for (const auto& cardJson : cardsJson) {
        try {
            Card card = Card::cardFromJson(cardJson, deckSlotPos, deckCardSize, textureManager);
            allCards[card.getName()] = card;
            std::cout << "Caricata carta: " << card.getName() << std::endl;
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
    try {
        deckFile >> decksJson;
    } catch (const std::exception& e) {
        throw std::runtime_error("Errore nel parsing del JSON dei deck: " + std::string(e.what()));
    }
    
    // Controlla se il JSON contiene un array di deck o un singolo deck
    if (decksJson.is_array()) {
        for (const auto& deckJson : decksJson) {
            try {
                Deck deck = Deck::deckFromJson(deckJson, allCards, deckSlotPos, deckCardSize, slotSize, textureFlipped);
                availableDecks.push_back(deck);
                std::cout << "Caricato deck: " << deck.getDeckName() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "ERRORE nella creazione del deck: " << e.what() << std::endl;
            }
        }
    } else if (decksJson.is_object()) {
        // Gestione del caso in cui il JSON contenga un singolo deck
        try {
            Deck deck = Deck::deckFromJson(decksJson, allCards, deckSlotPos, deckCardSize, slotSize, textureFlipped);
            availableDecks.push_back(deck);
            std::cout << "Caricato deck singolo: " << deck.getDeckName() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "ERRORE nella creazione del deck singolo: " << e.what() << std::endl;
        }
    } else {
        throw std::runtime_error("Il JSON dei deck deve essere un array o un oggetto");
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