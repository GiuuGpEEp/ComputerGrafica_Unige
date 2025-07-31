#include "Deck.h"

Deck::Deck(sf::Vector2f pos, sf::Vector2f deckCardSize, sf::Texture& textureRef, int numberDeckCards)
    : cardsSize(numberDeckCards){

        cards.reserve(numberDeckCards + 5);

        for(int i = 0; i < numberDeckCards; ++i) {
             // Calcola il centro dello slot con un leggero aggiustamento verso il basso
            
        }


}        