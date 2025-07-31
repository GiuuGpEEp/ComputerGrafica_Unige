#include "Deck.h"

Deck::Deck(sf::Vector2f pos, sf::Vector2f deckCardSize, sf::Vector2f slotSize, sf::Texture& texture, int numberDeckCards)
    : cardsSize(numberDeckCards){

        cards.reserve(numberDeckCards + 5);

        sf::Vector2f lastPos = slotCenter - sf::Vector2f(deckCardSize.x / 2.f, deckCardSize.y / 2.f) + offset;

        for(int i = 0; i < numberDeckCards-1; ++i) {
             // Calcola il centro dello slot con un leggero aggiustamento verso il basso
            sf::Vector2f slotCenter = pos + sf::Vector2f(slotSize.x / 2.f, slotSize.y / 2.f + 5.f);
        
        // Applica l'offset di volta in volta per ogni carta per l'effetto di impilamento (carta 0 = top, carta 24 = bottom)
            sf::Vector2f offset(0.f, -1.f * (24 - i));
        
        // Centra la carta scalata rispetto al centro dello slot con offset
            sf::Vector2f centeredPos = slotCenter - sf::Vector2f(deckCardSize.x / 2.f, deckCardSize.y / 2.f) + offset;
        
            cards.emplace_back("Carta " + std::to_string(i + 1), "Descrizione della carta " + std::to_string(i + 1), 
                          1000, 800, centeredPos, deckCardSize, texture);
        }
        cards.emplace_back("Drago Bianco", "Questo drago leggendario e' una potente macchina distruttrice. Virtualmente invincibile, sono in pochi ad aver fronteggiato questa creatura ed essere sopravvissuti per raccontarlo.", 
                       3000, 2500, centeredPos, cardSize, textureNonFlipped);

}

Card Deck::drawCard(int drawCards) {
    if(cardsSize <= 0) {
        throw std::out_of_range("Deck is empty, cannot draw a card.");
    }

    Card drawnCard = cards.back();
    cards.pop_back();
    --cardsSize;

    return drawnCard;
}


int Deck::getSize() const {
    return static_cast<int>(cards.size());
}

void Deck::draw(sf::RenderWindow& window){
    for(int i = 0; i < static_cast<int>(cards.size()); ++i) 
        cards[cards.size() - 1 - i].draw(window);
}


