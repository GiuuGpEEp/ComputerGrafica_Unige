#pragma once //Ho la garanzia che questo file non venga incluso più di una volta
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include "../../resources/data/GameState.h"
#include "../../resources/data/Type.h"
#include "../../resources/data/Attribute.h"
#include "../../resources/data/Feature.h"
#include <optional>
#include "../../resources/jsonData/json.hpp"
#include "../TextureManager/TextureManager.h"

class Card {
public:
    Card(); // Costruttore di default necessario per STL
    static Card cardFromJson(const nlohmann::json& jsonData,  sf::Vector2f pos, sf::Vector2f size, TextureManager& textureManager);
    static Type stringToType(const std::string& str);
    static Attribute stringToAttribute(const std::string& str);
    static Feature stringToFeature(const std::string& str);
    static std::vector<Feature> parseAllFeatures(const nlohmann::json& jsonData);

    Card(const std::string& name, const std::string& description, std::optional <int> atk, std::optional <int> def, sf::Vector2f pos, sf::Vector2f size, sf::Texture& textureRef, Type type, Attribute attribute, std::optional <int> level, std::vector<Feature> features = {});
    Card(sf::Texture& texture); 
    ~Card(); // Distruttore esplicito della classe 

    void draw(sf::RenderWindow& window, sf::Color color = sf::Color::White);
    bool isClicked(const sf::Vector2i& mousePos);
    void setPosition(sf::Vector2f pos);
    void setTexture(sf::Texture& texture);
    void setTextureRect(const sf::IntRect& rect);
    void setSize(sf::Vector2f size);
    void setRotation(float degrees);
    void setOrigin(sf::Vector2f origin);
    void centerOrigin();
    void resetOrigin();
    std::string getName() const;
    std::string getDescription() const;
    std::optional<std::pair<int, int>> getValues() const;
    sf::Vector2f& getPositionRef(); // Restituisce una reference alla posizione della carta
    sf::Vector2f getPosition() const; // Restituisce la posizione della carta
    sf::FloatRect getGlobalBounds() const; // Bordi globali dello sprite (se presente)
    void setOffset(float offset);
    float getOffset() const;
    Type getType() const;
    Attribute getAttribute() const;
    std::optional<int> getLevelOrRank() const;
    const std::vector<Feature>& getFeatures() const;

public:
    std::string originalTexturePath;
private:
    std::string name, description;
    std::optional <int> attack, defense, level, rank;
    std::optional<sf::Sprite> sprite; // ora opzionale
    sf::Vector2f position;
    float offset = 0.f;
    Type type;
    Attribute attribute;
    std::vector<Feature> features;
};
