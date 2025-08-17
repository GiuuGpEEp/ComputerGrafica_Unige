#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "../TextureManager/TextureManager.h"
#include "../ResourceManager/ResourceManager.h"

class DeckSelectionScreen {
public:
    DeckSelectionScreen(sf::RenderWindow& window, TextureManager& textureManager, ResourceManager& resourceManager, const sf::Texture& backgroundTexture, sf::Vector2u windowSize, sf::Font& font);
    void update();
    void draw();
    void drawSelectedDeckText(const sf::Font& font);
    int getHoveredDeckIndex(const sf::Vector2i& mousePos) const;
    int getSelectedDeckIndex() const;
    void setSelectedDeckIndex(int idx);
    std::string getSelectedDeckName() const;
private:
    sf::RenderWindow& window;
    TextureManager& textureManager;
    ResourceManager& resourceManager;
    const sf::Texture& backgroundTexture;
    sf::Vector2u windowSize;
    std::vector<sf::Sprite> deckSprites;
    std::vector<std::string> deckNames;
    sf::Font& font;
    int hoveredDeckIndex = -1;
    int selectedDeckIndex = -1;
    // Puoi aggiungere qui la gestione delle particelle come nella HomePage
};
