 #include "Deck.h"

Deck::Deck(sf::Vector2f pos, sf::Vector2f deckCardSize, sf::Vector2f slotSize, sf::Texture& texture, int numberDeckCards)
    : cardsSize(numberDeckCards){

        cards.reserve(numberDeckCards + 5);

        sf::Vector2f lastPos;

        for(int i = 0; i < numberDeckCards-1; ++i) {
             // Calcola il centro dello slot con un leggero aggiustamento verso il basso
            sf::Vector2f slotCenter = pos + sf::Vector2f(slotSize.x / 2.f, slotSize.y / 2.f + 5.f);
        
            // Applica l'offset di volta in volta per ogni carta per l'effetto di impilamento (carta 0 = top, carta 24 = bottom)
            sf::Vector2f offset(0.f, -1.f * (24 - i));
        
            // Centra la carta scalata rispetto al centro dello slot con offset
            sf::Vector2f centeredPos = slotCenter - sf::Vector2f(deckCardSize.x / 2.f, deckCardSize.y / 2.f) + offset;
        
            cards.emplace_back("Carta " + std::to_string(i + 1), "Descrizione della carta " + std::to_string(i + 1), 
                          1000, 800, centeredPos, deckCardSize, texture, Type::Monster);
            lastPos = centeredPos;
        }
        cards.emplace_back("Drago Bianco", "Questo drago leggendario e' una potente macchina distruttrice. Virtualmente invincibile, sono in pochi ad aver fronteggiato questa creatura ed essere sopravvissuti per raccontarlo.", 
                       3000, 2500, lastPos, deckCardSize, texture);

}

Card Deck::drawCard() {
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

bool Deck::isEmpty() const {
    return cards.empty();
}

void Deck::draw(sf::RenderWindow& window, const sf::Vector2i& mousePos, const sf::Font& font, sf::Vector2f slotPos, sf::Vector2f slotSize, GameState gamestate){

    if(gamestate != GameState::FieldVisible && gamestate != GameState::Playing) {
        return;
    }

    sf::Color alphaColor(255, 255, 255, static_cast<uint8_t>(deckAlpha));

    // Disegna le carte del deck
    for(int i = 0; i < static_cast<int>(cards.size()); ++i) 
        cards[cards.size() - 1 - i].draw(window, alphaColor);
    
    // Disegna il testo con il numero di carte solo se il mouse è sopra l'area del deck
    sf::Vector2f mouseFloat(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    sf::FloatRect deckArea(slotPos, slotSize);
    
    if (deckArea.contains(mouseFloat) && gamestate == GameState::Playing) {
        sf::Text deckLabel(font, "", 16);
        deckLabel.setFillColor(sf::Color::White);
        deckLabel.setStyle(sf::Text::Bold);
        deckLabel.setString(std::to_string(getSize()));
        
        // Calcola il centro del deck per posizionare il testo
        sf::Vector2f deckCenter = slotPos + sf::Vector2f(slotSize.x / 2.f, slotSize.y / 2.f + 5.f);
        sf::FloatRect textBounds = deckLabel.getLocalBounds();
        deckLabel.setPosition(deckCenter - sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 0.5f));
        window.draw(deckLabel);
    }
}

void Deck::setAnimationFinished() {
    animationFinished = true; 
}

bool Deck::isAnimationFinished() const{
    return animationFinished; 
}

void Deck::animate(float deltaTime){
    float fadeSpeed = 200.f;

    deckAlpha += fadeSpeed * deltaTime;
    if(deckAlpha >= 255.f) deckAlpha = 255.f;

    if(deckAlpha == 255.f) setAnimationFinished();
}

