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
    void draw(sf::RenderWindow& window);
    
private:
    TextureManager& textureManager;
    ResourceManager& resourceManager;
    const sf::Texture& backgroundTexture;
    std::vector<sf::Sprite> deckSprites;
    std::vector<std::string> deckNames;
    sf::Font& font;
    std::vector<sf::RectangleShape> deckRects;
    std::vector<sf::Text> deckLabelsText;

    // Campi per animazione particelle
        std::vector<sf::CircleShape> particles;
        std::vector<float> particleAlpha;
        std::vector<sf::Vector2f> particleVel;
        sf::Clock particleClock;
        sf::Clock spawnClock;
         // Metodo privato per inizializzare/azzerare le particelle
        void initParticles();
  
    
};
