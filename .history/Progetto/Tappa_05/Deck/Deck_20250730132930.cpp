#include "Deck.h"

Deck::Deck(sf::Vector2f pos, sf::Vector2f deckCardSize, sf::Vector2f slotSize, sf::Texture& texture, int numberDeckCards)
    : cardsSize(numberDeckCards){

        cards.reserve(numberDeckCards + 5);

        for(int i = 0; i < numberDeckCards; ++i) {
             // Calcola il centro dello slot con un leggero aggiustamento verso il basso
            sf::Vector2f slotCenter = pos + sf::Vector2f(slotSize.x / 2.f, slotSize.y / 2.f + 5.f);
        
        // Applica l'offset di volta in volta per ogni carta per l'effetto di impilamento (carta 0 = top, carta 24 = bottom)
            sf::Vector2f offset(0.f, -1.f * (24 - i));
        
        // Centra la carta scalata rispetto al centro dello slot con offset
            sf::Vector2f centeredPos = slotCenter - sf::Vector2f(deckCardSize.x / 2.f, deckCardSize.y / 2.f) + offset;
        
            cards.emplace_back("Carta " + std::to_string(i + 1), "Descrizione della carta " + std::to_string(i + 1), 
                          1000, 800, centeredPos, deckCardSize, texture);
        }

}

void Deck::draw