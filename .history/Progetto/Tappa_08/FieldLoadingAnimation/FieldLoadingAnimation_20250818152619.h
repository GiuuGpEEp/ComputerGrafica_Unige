#pragma once
#include <SFML/Graphics.hpp>

class FieldLoadingAnimation {
public:
    FieldLoadingAnimation();
    void start(const sf::Texture& homeTex, const sf::Texture& fieldTex, sf::Vector2u windowSize);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    bool isFinished() const;
    bool hasStarted() const { return started; }
private:
    sf::Sprite homeSprite;
    sf::Sprite fieldSprite;
    float duration{2.0f}; // durata in secondi
    float elapsed{0.f};
    bool started{false};
};
