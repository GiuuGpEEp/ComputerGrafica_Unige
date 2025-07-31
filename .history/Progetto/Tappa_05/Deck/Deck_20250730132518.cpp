#include "Deck.h"

Deck::Deck(sf::Vector2f pos, sf::Vector2f deckCardSize, sf::Texture& textureRef, int numberDeckCards)
    : cardsSize(numberDeckCards){

        cards.reserve(numberDeckCards + 5);

        for(int i = 0; i < numberDeckCards; ++i) {
            
            // Calcola la posizione per ogni carta nel deck
            sf::Vector2f offset(0.f, static_cast<float>(i) * 10.f); // Offset per l'effetto di impilamento
            sf::Vector2f centeredPos = pos - sf::Vector2f(deckCardSize.x / 2.f, deckCardSize.y / 2.f) + offset;

            // Crea una carta e aggiungila al deck
            cards.emplace_back("Card " + std::to_string(i + 1), "Description of Card " + std::to_string(i + 1), 
                               rand() % 100, rand() % 100, centeredPos, deckCardSize, textureRef);
        }


}        