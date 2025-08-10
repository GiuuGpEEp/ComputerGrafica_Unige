//Definisco cosa fa la classe Card

#include "Card.h"

Card::Card(const std::string& name, const std::string& description, int atk, int def, sf::Vector2f pos, sf::Vector2f size, sf::Texture& textureRef)
    : name(name), description(description), attack(atk), defense(def), sprite(textureRef), position(pos) //Inizializzo sprite con textureRef e position
{
    //Inizializzo lo sprite della carta
    sprite.setTexture(textureRef);

    // Imposto le dimensioni desiderate per lo sprite della carta usando il parametro size
    sf::FloatRect bounds = sprite.getLocalBounds();
    sf::Vector2f scale(size.x / bounds.size.x, size.y / bounds.size.y);
    sprite.setScale(scale);
    
    sprite.setPosition(pos);
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
std::pair<int, int> Card::getValues() const {
    return {attack, defense};
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

Type Card::getType