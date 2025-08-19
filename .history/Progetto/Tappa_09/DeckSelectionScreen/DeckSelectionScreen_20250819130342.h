#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "../TextureManager/TextureManager.h"
#include "../ResourceManager/ResourceManager.h"

class DeckSelectionScreen {
public:
    DeckSelectionScreen(sf::RenderWindow& window, TextureManager& textureManager, ResourceManager& resourceManager, const sf::Texture& backgroundTexture, sf::Vector2u windowSize, sf::Font& font);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    void drawSelectedText(sf::RenderWindow& window);
    sf::FloatRect getDeckBlueEyesBounds() const;
    sf::FloatRect getDeckDarkMagicianBounds() const;
    sf::FloatRect getDeckRedEyesBounds() const;
    std::string getSelectedDeckName() const;
    
    void setSelectedDeck(int index);
    bool hasSelection() const;
    
    // Fade API
    void startFadeIn(float duration = 0.5f);
    void startFadeOut(float duration = 0.5f);
    bool isFadeOutFinished() const { return fadeMode == FadeMode::None && fadeOutCompleted; }
    bool isFading() const { return fadeMode != FadeMode::None; }
    float getFadeAlpha() const { return fadeAlpha; }
    

private:
    TextureManager& textureManager;
    ResourceManager& resourceManager;
    std::vector<sf::Sprite> deckSprites;
    std::vector<std::string> deckNames;
    sf::Font& font;
    std::vector<sf::RectangleShape> deckRects;
    std::vector<sf::Text> deckLabelsText;
    sf::Sprite backgroundSprite;
    sf::Text titleText; // Titolo schermata
    sf::Text selectionText; // Testo conferma selezione
    int selectedDeckIndex = -1;

    // Campi per animazione particelle
    std::vector<sf::CircleShape> particles;
    std::vector<float> particleAlpha;
    std::vector<sf::Vector2f> particleVel;
    sf::Clock particleClock;
    sf::Clock spawnClock;
    // Fade state
    enum class FadeMode { None, In, Out };
    FadeMode fadeMode = FadeMode::None;
    float fadeAlpha = 0.f; // 0 visibile (schermata), 255 oscurata
    float fadeDuration = 0.5f;
    float fadeTimer = 0.f;
    bool fadeOutCompleted = false;
        
    // Metodo privato per inizializzare/azzerare le particelle
    void initParticles();
  
    
};
