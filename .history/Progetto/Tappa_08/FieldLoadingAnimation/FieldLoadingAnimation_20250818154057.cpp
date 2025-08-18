#include "FieldLoadingAnimation.h"
#include <cmath>

FieldLoadingAnimation::FieldLoadingAnimation(const sf::Texture& homeTex, const sf::Texture& fieldTex, sf::Vector2u windowSize) 
    : homeSprite(homeTex), fieldSprite(fieldTex) {

    homeSprite.setScale(sf::Vector2f(
        float(windowSize.x) / homeTex.getSize().x,
        float(windowSize.y) / homeTex.getSize().y
    ));
    fieldSprite.setScale(sf::Vector2f(
        float(windowSize.x) / fieldTex.getSize().x,
        float(windowSize.y) / fieldTex.getSize().y
    ));
}

void FieldLoadingAnimation::start(const sf::Texture& homeTex, const sf::Texture& fieldTex, sf::Vector2u windowSize){
    elapsed = 0.f;
    started = true;
}

void FieldLoadingAnimation::update(float dt){
    if(!started) return;
    elapsed += dt;
    if(elapsed > duration) elapsed = duration;
}

void FieldLoadingAnimation::draw(sf::RenderWindow& window){
    if(!started) return;
    float t = (duration > 0.f) ? (elapsed / duration) : 1.f; // 0..1
    // Ease in-out cubic
    float tt = (t < 0.5f) ? 4.f * t * t * t : 1.f - std::pow(-2.f * t + 2.f, 3.f) / 2.f;

    // Cross-fade
    sf::Color homeColor = sf::Color::White; homeColor.a = static_cast<uint8_t>(255 * (1.f - tt));
    sf::Color fieldColor = sf::Color::White; fieldColor.a = static_cast<uint8_t>(255 * tt);

    // Zoom leggero
    float homeScaleFactor = 1.f - 0.05f * tt; // da 1.00 a 0.95
    float fieldScaleFactor = 1.05f - 0.05f * tt; // da 1.05 a 1.00

    sf::Vector2f windowCenter(window.getSize().x / 2.f, window.getSize().y / 2.f);
    sf::Texture homeTexture = homeSprite.getTexture();
    sf::Texture fieldTexture = fieldSprite.getTexture();

    auto sizeH = homeTexture.getSize();
    auto sizeF = fieldTexture.getSize();
    homeSprite.setScale(sf::Vector2f(float)window.getSize().x / sizeH.x * homeScaleFactor,
                        (float)window.getSize().y / sizeH.y * homeScaleFactor);
    fieldSprite.setScale((float)window.getSize().x / sizeF.x * fieldScaleFactor,
                         (float)window.getSize().y / sizeF.y * fieldScaleFactor);

    sf::FloatRect hBounds = homeSprite.getGlobalBounds();
    sf::FloatRect fBounds = fieldSprite.getGlobalBounds();
    homeSprite.setPosition(windowCenter.x - hBounds.width / 2.f, windowCenter.y - hBounds.height / 2.f);
    fieldSprite.setPosition(windowCenter.x - fBounds.width / 2.f, windowCenter.y - fBounds.height / 2.f);

    homeSprite.setColor(homeColor);
    fieldSprite.setColor(fieldColor);

    window.draw(homeSprite);
    window.draw(fieldSprite);
}

bool FieldLoadingAnimation::isFinished() const {
    return started && elapsed >= duration;
}
