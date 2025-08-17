// Includo Card.h prima di tutto
#include "Card.h"
#include <filesystem>

// Costruttore di default
Card::Card()
    : name("")
    , description("")
    , attack(std::nullopt)
    , defense(std::nullopt)
    , level(std::nullopt)
    , rank(std::nullopt)
    , sprite(std::nullopt) // Nessuno sprite di default
    , position(0.f, 0.f)
    , offset(0.f)
    , type(Type::FieldSpell)
    , attribute(Attribute::None)
    , features()
{
    // Lo sprite verrà creato solo quando serve
}

Card Card::cardFromJson(const nlohmann::json& jsonData, sf::Vector2f pos, sf::Vector2f size, TextureManager& textureManager) {

    std::string name = jsonData["name"];
    std::string description = jsonData["description"];

    std::optional<int> attack = std::nullopt;
    if(jsonData.contains("attack")) {
        attack = jsonData["attack"];
    }

    std::optional<int> defense = std::nullopt;
    if(jsonData.contains("defense")) {
        defense = jsonData["defense"];
    }

    // Ricerca la texture nelle cartelle dei deck (supporto multi-deck)
    const std::string basePath = "../../Progetto/resources/textures/";
    const std::vector<std::string> deckFolders = {"textureDeck1/", "textureDeck2/"};
    sf::Texture* chosenTexture = nullptr;
    std::string chosenPath;
    std::string fileName = jsonData["texture"].get<std::string>();

    for(const auto& folder : deckFolders){
        std::string fullPath = basePath + folder + fileName;
        if(std::filesystem::exists(fullPath)){
            chosenTexture = &textureManager.getTexture(fullPath);
            chosenPath = fullPath;
            break;
        }
    }

    if(!chosenTexture){
        // Se non trovata in nessuna cartella, prova comunque a caricare dalla prima (genererà messaggio di errore interno) e continua
        std::string fallbackPath = basePath + deckFolders.front() + fileName;
        chosenTexture = &textureManager.getTexture(fallbackPath);
        chosenPath = fallbackPath;
        std::cerr << "ATTENZIONE: Texture " << fileName << " non trovata in cartelle note, uso fallback: " << fallbackPath << std::endl;
    }
    sf::Texture& texture = *chosenTexture;
    Type type = stringToType(jsonData["type"]);
    Attribute attribute = stringToAttribute(jsonData["attribute"]);

    std::optional<int> level = std::nullopt;
    if(jsonData.contains("level")) {
        level = jsonData["level"];
    }

    std::optional<int> rank = std::nullopt;
    if(jsonData.contains("rank")) {
        rank = jsonData["rank"];
    }

    std::vector<Feature> features = parseAllFeatures(jsonData["features"]);

    Card card(name, description, attack, defense, pos, size, texture, type, attribute, level, features);
    card.originalTexturePath = chosenPath;
    return card;
}

Type Card::stringToType(const std::string& str) {
    if (str == "Mostro") return Type::Monster;
    if (str == "Magia") return Type::SpellTrap;
    if (str == "Trappola") return Type::SpellTrap;
    if (str == "Terreno") return Type::FieldSpell;
    throw std::invalid_argument("Invalid type string");
}

Attribute Card::stringToAttribute(const std::string& str) {
    if (str == "Luce") return Attribute::Luce;
    if (str == "Oscurita" ) return Attribute::Oscurita;
    if (str == "Vento") return Attribute::Vento;
    if (str == "Acqua") return Attribute::Acqua;
    if (str == "Fuoco") return Attribute::Fuoco;
    if (str == "Terra") return Attribute::Terra;
    if (str == "Magia") return Attribute::Magia;
    if (str == "Trappola") return Attribute::Trappola;
    if (str == "Nessuno") return Attribute::None;
    throw std::invalid_argument("Invalid attribute string");
}

Feature Card::stringToFeature(const std::string& str) {
    if (str == "Fusione") return Feature::Fusione;
    if (str == "Rituale") return Feature::Rituale;
    if (str == "Synchro") return Feature::Synchro;
    if (str == "Xyz") return Feature::Xyz;
    if (str == "Normale") return Feature::Normale;
    if (str == "Effetto") return Feature::Effetto;
    if (str == "Aqua") return Feature::Aqua;
    if (str == "Bestia") return Feature::Bestia;
    if (str == "Bestia Alata") return Feature::BestiaAlata;
    if (str == "Bestia Guerriero") return Feature::BestiaGuerriero;
    if (str == "Fata") return Feature::Fata;
    if (str == "Drago") return Feature::Drago;
    if (str == "Dinosauro") return Feature::Dinosauro;
    if (str == "Divinità Bestia") return Feature::DivinitaBestia;
    if (str == "Incantatore") return Feature::Incantatore;
    if (str == "Insetto") return Feature::Insetto;
    if (str == "Macchina") return Feature::Macchina;
    if (str == "Pesce") return Feature::Pesce;
    if (str == "Pianta") return Feature::Pianta;
    if (str == "Pyro") return Feature::Pyro;
    if (str == "Rettile") return Feature::Rettile;
    if (str == "Roccia") return Feature::Roccia;
    if (str == "Demone") return Feature::Demone;
    if (str == "Serpente Marino") return Feature::SerpenteMarino;
    if (str == "Tuono") return Feature::Tuono;
    if (str == "Guerriero") return Feature::Guerriero;
    if (str == "Zombie") return Feature::Zombie;
    if (str == "Psichico") return Feature::Psichico;
    if (str == "Tuner") return Feature::Tuner;
    if (str == "Rapida") return Feature::Rapida;
    if (str == "Continua") return Feature::Continua;
    if (str == "Terreno") return Feature::Terreno;
    if (str == "ControTrappola") return Feature::ControTrappola;
    throw std::invalid_argument("Invalid feature string");
}

std::vector<Feature> Card::parseAllFeatures(const nlohmann::json& jsonData){
    std::vector<Feature> features;
    for (const auto& feature : jsonData) {
        features.push_back(stringToFeature(feature));
    }
    return features;
}

Card::Card(const std::string& name, const std::string& description, std::optional<int> atk, std::optional<int> def, sf::Vector2f pos, sf::Vector2f size, sf::Texture& textureRef, Type type, Attribute attribute, std::optional<int> level, std::vector<Feature> cardFeatures)
    : name(name), description(description), attack(atk), defense(def), sprite(sf::Sprite(textureRef)), position(pos), type(type), attribute(attribute), features(cardFeatures)
{
    //Inizializzo lo sprite della carta
    sprite->setTexture(textureRef);

    // Imposto le dimensioni desiderate per lo sprite della carta usando il parametro size
    sf::FloatRect bounds = sprite->getLocalBounds();
    sf::Vector2f scale(size.x / bounds.size.x, size.y / bounds.size.y);
    sprite->setScale(scale);
    
    sprite->setPosition(pos);

    //Le carte xyz non hanno livello ma rango 
    if(std::find(cardFeatures.begin(), cardFeatures.end(), Feature::Xyz) != cardFeatures.end()){
        this->rank = level;
    }
    else {
        this->level = level;
    }
}

Card::Card(sf::Texture& texture)
    : name("")
    , description("")
    , attack(0)
    , defense(0)
    , position(0.f, 0.f)
    , offset(0.f)
    , type(Type::Monster)
    , attribute(Attribute::None)
    , sprite(sf::Sprite(texture))
{
    // Sprite creato con texture
}

//Distruttore
Card::~Card() {}

//Disegno la carta
void Card::draw(sf::RenderWindow& window, sf::Color color) {
    if (sprite.has_value()) {
        sprite->setColor(color);
        window.draw(*sprite);
    }
}

//Controllo se la carta è stata cliccata - Verifico se il mouse è all'interno della forma della carta
bool Card::isClicked(const sf::Vector2i& mousePos) {
    if (sprite.has_value()) {
        return sprite->getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
    }
    return false;
}

//Imposto la posizione della carta
void Card::setPosition(sf::Vector2f pos) {
    if (sprite.has_value()) sprite->setPosition(pos);
    position = pos;
}

//Cambio la texture della carta
void Card::setTexture(sf::Texture& texture) {
    if (sprite.has_value()) sprite->setTexture(texture);
    else sprite = sf::Sprite(texture);
}

//Imposto il rettangolo di texture per lo sprite
void Card::setTextureRect(const sf::IntRect& rect) {
    if (sprite.has_value()) sprite->setTextureRect(rect);
}

//Cambio le dimensioni della carta
void Card::setSize(sf::Vector2f size) {
    if (sprite.has_value()) {
        const sf::Texture& currentTexture = sprite->getTexture();
        sf::Vector2u textureSize = currentTexture.getSize();
        sprite->setScale(sf::Vector2f(size.x / textureSize.x, size.y / textureSize.y));
    }
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

