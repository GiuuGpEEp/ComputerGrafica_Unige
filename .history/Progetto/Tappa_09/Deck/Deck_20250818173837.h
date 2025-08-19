#pragma once //Ho la garanzia che questo file non venga incluso più di una volta
#include <SFML/Graphics.hpp>
#include "../Card/Card.h"
#include "ShuffleAnimation.h"
#include "../../resources/data/GameState.h"
#include <random>
#include <vector>

enum class ShufflePhase {
    LateralMove,
    Overlap,
    Return
};

class Deck{
    
    public:
        static Deck deckFromJson(const nlohmann::json& deckJson,
                 const std::unordered_map<std::string, Card>& allCards,
                 sf::Vector2f pos, sf::Vector2f deckCardSize, sf::Vector2f slotSize,
                 sf::Texture& textureFlipped);

        Deck(const std::vector<Card>& cards, const std::vector<Card>& extraCards,
            sf::Vector2f deckSlotPos,  sf::Vector2f deckCardSize, sf::Vector2f slotSize,
            sf::Texture& textureFlipped, std::string deckName);
        void shuffle();
        void resetDeckCardPositions(sf::Vector2f deckPos, sf::Vector2f deckCardSize, sf::Vector2f slotSize, sf::Texture& textureFlipped);
        Card drawCard();
        int getSize() const;
        int getExtraSize() const;
        bool isEmpty() const;
        void draw(sf::RenderWindow& window, const sf::Vector2i& mousePos, const sf::Font& font, sf::Vector2f slotPos, sf::Vector2f slotSize, GameState gamestate);
        void drawExtra(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f extraSlotPos, sf::Vector2f slotSize, GameState gamestate);
        bool isAnimationFinished() const;
        void setAnimationFinished();
        void animate(float deltaTime);
        std::string getDeckName() const;

        // Costruttore di default
        Deck();
        // Costruttore di copia
        Deck(const Deck& other);
        // Operatore di assegnazione per inserire un deck in una map
        Deck& operator=(const Deck& other);

        // Animazione di mischiata avanzata
        void startShuffleAnimationAdvanced(sf::Vector2f deckPos, sf::Vector2f deckCardSize);
        void updateShuffleAnimationAdvanced(float dt);
        void drawShuffleAnimationAdvanced(sf::RenderWindow& window);
        bool isShuffleAnimationAdvancedFinished() const;
        void clearShuffleAnimationAdvanced();

    private:
        float deckAlpha = 0.f;
        std::vector<Card> cards;
        std::vector<Card> extraCards;
        int cardsSize;
        bool animationFinished;
        std::string deckName;

        // ShuffleAnimation avanzata
        std::unique_ptr<ShuffleAnimation> shuffleAnimation = nullptr;

};