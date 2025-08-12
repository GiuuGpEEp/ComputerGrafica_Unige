//Definisco cosa fa la classe Card

#include "Card.h"

Card::Card(const std::string& name, const std::string& description, std::optional<int> atk, std::optional<int> def, sf::Vector2f pos, sf::Vector2f size, sf::Texture& textureRef, Type type, Attribute attribute, std::optional<int> level, std::vector<Feature> cardFeatures)
    : name(name), description(description), attack(atk), defense(def), sprite(textureRef), position(pos), type(type), attribute(attribute), features(cardFeatures)
{
    //Inizializzo lo sprite della carta
    sprite.setTexture(textureRef);

    // Imposto le dimensioni desiderate per lo sprite della carta usando il parametro size
    sf::FloatRect bounds = sprite.getLocalBounds();
    sf::Vector2f scale(size.x / bounds.size.x, size.y / bounds.size.y);
    sprite.setScale(scale);
    
    sprite.setPosition(pos);

    //Le carte xyz non hanno livello ma rango 
    if(std::find(cardFeatures.begin(), cardFeatures.end(), Feature::Xyz) != cardFeatures.end()){
        this->rank = level;
    }
    else {
        this->level = level;
    }
}

//Distruttore
Card::~Card() {
    // Distruttore pulito - debug rimosso
}

//Disegno la carta
void Card::draw(sf::RenderWindow& window, sf::Color color) {
    sprite.setColor(color);
    window.draw(sprite);
}

//Controllo se la carta è stata cliccata - Verifico se il mouse è all'interno della forma della carta
bool Card::isClicked(const sf::Vector2i& mousePos) {
    return sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

//Imposto la posizione della carta
void Card::setPosition(sf::Vector2f pos) {
    sprite.setPosition(pos);
    position = pos;
}

//Cambio la texture della carta
void Card::setTexture(sf::Texture& texture) {
    sprite.setTexture(texture);
}

//Imposto il rettangolo di texture per lo sprite
void Card::setTextureRect(const sf::IntRect& rect) {
    sprite.setTextureRect(rect);
}

//Cambio le dimensioni della carta
void Card::setSize(sf::Vector2f size) {
    const sf::Texture& currentTexture = sprite.getTexture();
    sf::Vector2u textureSize = currentTexture.getSize();
    sprite.setScale(sf::Vector2f(size.x / textureSize.x, size.y / textureSize.y));
}

//Restituisco il nome della carta
std::string Card::getName() const {
    return name;
}

//Restituisco la descrizione della carta
std::string Card::getDescription() const {
    return description;
}

//Restituisco i valori di attacco e difesa della carta (si usa const per garantire che la funzione non modifichi lo stato dell'oggetto)
std::optional<std::pair<int, int>> Card::getValues() const {
    return std::make_optional(std::make_pair(attack.value_or(0), defense.value_or(0)));
}

sf::Vector2f& Card::getPositionRef() {
    return position;
}

sf::Vector2f Card::getPosition() const {
    return position;
}

void Card::setOffset(float offset) {
    this->offset = offset;
}

float Card::getOffset() const {
    return offset;
}

Type Card::getType() const{
    return type;
}

Attribute Card::getAttribute() const {
    return attribute;
}

std::optional <int> Card::getLevelOrRank() const {
    if (level.has_value()) {
        return level.value();
    }
    return rank.value_or(0);
}

const std::vector<Feature>& Card::getFeatures() const {
    return features;
}