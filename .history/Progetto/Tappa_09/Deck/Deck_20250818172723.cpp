#include "Deck.h"

// Costruttore di default
Deck::Deck()
    : cards(), extraCards(), cardsSize(0), animationFinished(true), deckName("")
{
    // shuffleAnimation rimane nullptr
}

// Costruttore di copia
Deck::Deck(const Deck& other)
    : cards(other.cards), extraCards(other.extraCards), cardsSize(other.cardsSize), animationFinished(other.animationFinished), deckName(other.deckName)
{
    shuffleAnimation = nullptr;
}

// Operatore di assegnazione
Deck& Deck::operator=(const Deck& other)
{
    if (this != &other) {
    cards = other.cards;
    extraCards = other.extraCards;
        cardsSize = other.cardsSize;
        animationFinished = other.animationFinished;
        deckName = other.deckName;
        shuffleAnimation = nullptr; // Non copiare la shuffleAnimation
    }
    return *this;
}

Deck Deck::deckFromJson(const nlohmann::json& deckJson,
                        const std::unordered_map<std::string, Card>& allCards,
                     sf::Vector2f pos, sf::Vector2f deckCardSize, sf::Vector2f slotSize,
                        sf::Texture& textureFlipped)
{
    std::vector<Card> deckCards;
    std::vector<Card> extra;
    // Ora deckJson è direttamente l'oggetto deck
    if (!deckJson.is_object()) {
        throw std::runtime_error("Il JSON del deck non è un oggetto.");
    }
    if (!deckJson.contains("cards")) {
        throw std::runtime_error("Il JSON del deck non contiene la chiave 'cards'.");
    }
    for (const auto& cardName : deckJson["cards"]) {
        auto it = allCards.find(cardName);
        if (it != allCards.end()) {
            deckCards.push_back(it->second);
        }
    }
    // Extra deck opzionale (chiave "extra")
    if(deckJson.contains("extra")) {
        for(const auto& cardName : deckJson["extra"]) {
            auto it = allCards.find(cardName);
            if(it != allCards.end()) {
                extra.push_back(it->second);
            }
        }
    }
    if(!deckJson.contains("name")) {
        throw std::runtime_error("Il JSON del deck non contiene la chiave 'name'.");
    }
    std::string deckName = deckJson["name"];
    return Deck(deckCards, extra, pos, deckCardSize, slotSize, textureFlipped, deckName);
}



Deck::Deck(const std::vector<Card>& cards, const std::vector<Card>& extraCards,
        sf::Vector2f pos, sf::Vector2f deckCardSize, sf::Vector2f slotSize, sf::Texture& textureFlipped, std::string deckName)
    : cards(cards), extraCards(extraCards), cardsSize(static_cast<int>(cards.size())), animationFinished(false), deckName(deckName)
{
    sf::Vector2f lastPos;
    int numberDeckCards = static_cast<int>(this->cards.size());
    for (int i = 0; i < numberDeckCards - 1; ++i) {
        sf::Vector2f slotCenter = pos + sf::Vector2f(slotSize.x / 2.f, slotSize.y / 2.f + 5.f);
        sf::Vector2f offset(0.f, -1.f * (numberDeckCards - 1 - i));
        sf::Vector2f centeredPos = slotCenter - sf::Vector2f(deckCardSize.x / 2.f, deckCardSize.y / 2.f) + offset;
        this->cards[i].setPosition(centeredPos);
        this->cards[i].setTexture(textureFlipped);
        sf::Vector2u texSize = textureFlipped.getSize();
        this->cards[i].setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texSize.x, texSize.y)));
        this->cards[i].setSize(deckCardSize);
        lastPos = centeredPos;
    }
    // Ultima carta
    if (numberDeckCards > 0) {
        this->cards[numberDeckCards - 1].setPosition(lastPos);
        this->cards[numberDeckCards - 1].setTexture(textureFlipped);
        sf::Vector2u texSize = textureFlipped.getSize();
        this->cards[numberDeckCards - 1].setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texSize.x, texSize.y)));
        this->cards[numberDeckCards - 1].setSize(deckCardSize);
    }

    int numberExtraDeckCards = getExtraSize();
        


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

int Deck::getExtraSize() const {
    return static_cast<int>(extraCards.size());
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

void Deck::drawExtra(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f extraSlotPos, sf::Vector2f slotSize, GameState gamestate){
    if(gamestate != GameState::FieldVisible && gamestate != GameState::Playing) return;
    // Disegno semplice: pila compatta come il main deck (non animata) con alpha già raggiunto
    sf::Color alphaColor(255,255,255, static_cast<uint8_t>(deckAlpha));
    for(int i=0; i< static_cast<int>(extraCards.size()); ++i){
        extraCards[extraCards.size()-1-i].draw(window, alphaColor);
    }
    // Etichetta numero extra se mouse sopra (facoltativo: sempre mostrare?)
    sf::Text label(font, std::to_string(getExtraSize()), 14);
    label.setFillColor(sf::Color::White);
    sf::FloatRect bounds = label.getLocalBounds();
    label.setPosition(sf::Vector2f(extraSlotPos.x + slotSize.x/2.f - bounds.size.x/2.f, extraSlotPos.y + slotSize.y/2.f - bounds.size.y));
    window.draw(label);
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

void Deck::shuffle(){
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(cards.begin(), cards.end(), g);
}

// Animazione avanzata: inizializza ShuffleAnimation
void Deck::startShuffleAnimationAdvanced(sf::Vector2f deckPos, sf::Vector2f deckCardSize) {
    if (!shuffleAnimation) shuffleAnimation = std::make_unique<ShuffleAnimation>();
    shuffleAnimation->start(cards, deckPos, cards.size());
}

// Animazione avanzata: aggiorna ShuffleAnimation
void Deck::updateShuffleAnimationAdvanced(float deltaTime) {
    if (shuffleAnimation && !shuffleAnimation->isFinished()) {
        shuffleAnimation->update(deltaTime);
    }
}

// Animazione avanzata: disegna ShuffleAnimation
void Deck::drawShuffleAnimationAdvanced(sf::RenderWindow& window) {
    if (shuffleAnimation && !shuffleAnimation->isFinished()) {
        shuffleAnimation->draw(window);
    }
}

// Animazione avanzata: verifica se finita
bool Deck::isShuffleAnimationAdvancedFinished() const {
    return shuffleAnimation ? shuffleAnimation->isFinished() : true;
}

// Animazione avanzata: pulisci ShuffleAnimation
void Deck::clearShuffleAnimationAdvanced() {
    if (shuffleAnimation) shuffleAnimation->clear();
}

// Riallinea le carte del deck secondo la visualizzazione standard
void Deck::resetDeckCardPositions(sf::Vector2f deckPos, sf::Vector2f deckCardSize, sf::Vector2f slotSize, sf::Texture& textureFlipped) {
    sf::Vector2f lastPos;
    int numberDeckCards = static_cast<int>(cards.size());
    for (int i = 0; i < numberDeckCards - 1; ++i) {
        sf::Vector2f slotCenter = deckPos + sf::Vector2f(slotSize.x / 2.f, slotSize.y / 2.f + 5.f);
        sf::Vector2f offset(0.f, -1.f * (numberDeckCards - 1 - i));
        sf::Vector2f centeredPos = slotCenter - sf::Vector2f(deckCardSize.x / 2.f, deckCardSize.y / 2.f) + offset;
        cards[i].setPosition(centeredPos);
        cards[i].setTexture(textureFlipped);
        sf::Vector2u texSize = textureFlipped.getSize();
        cards[i].setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texSize.x, texSize.y)));
        cards[i].setSize(deckCardSize);
        lastPos = centeredPos;
    }
    if (numberDeckCards > 0) {
        cards[numberDeckCards - 1].setPosition(lastPos);
        cards[numberDeckCards - 1].setTexture(textureFlipped);
        sf::Vector2u texSize = textureFlipped.getSize();
        cards[numberDeckCards - 1].setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texSize.x, texSize.y)));
        cards[numberDeckCards - 1].setSize(deckCardSize);
    }
}

std::string Deck::getDeckName() const {
    return deckName;
}

