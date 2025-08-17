#pragma once
#include <SFML/Graphics.hpp>
#include "../auxFunc.h"
#include "../TextureManager/TextureManager.h"
#include <vector>

class HomePage{
    public:
        HomePage(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2u& windowSize, sf::Texture& texture, const std::string& deckName);
        void update();
        void draw(sf::RenderWindow& window);
        sf::FloatRect getGiocaBounds() const;
        sf::FloatRect getSelectionBounds() const;
        void setDeckName(const std::string& name);

    private:
        std::string deckName;
        sf::RenderWindow& window;
        sf::Sprite backgroundSprite;
        sf::Text menuText;
        std::vector<std::string> menuItems = {"Gioca", "Seleziona Deck"};
        std::vector<sf::Text> menuTexts;
        sf::Text deckText;
    
        // Campi per animazione particelle
        std::vector<sf::CircleShape> particles;
        std::vector<float> particleAlpha;
        std::vector<sf::Vector2f> particleVel;
        sf::Clock particleClock;
        sf::Clock spawnClock;

        // Metodo privato per inizializzare/azzerare le particelle
        void initParticles();

};