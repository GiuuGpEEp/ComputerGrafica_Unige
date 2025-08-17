#include "DeckSelectionScreen.h"
#include <iostream>

DeckSelectionScreen::DeckSelectionScreen(sf::RenderWindow& window, TextureManager& textureManager, ResourceManager& resourceManager, const sf::Texture& backgroundTexture, sf::Vector2u windowSize)
    : window(window), textureManager(textureManager), resourceManager(resourceManager), backgroundTexture(backgroundTexture), windowSize(windowSize)
{
    deckNames = resourceManager.getDeckNames();
    // Posiziona 3 deck centrali
    float spriteWidth = 250.f;
    float spriteHeight = 350.f;
    float spacing = 80.f;
    float totalWidth = deckNames.size() * spriteWidth + (deckNames.size() - 1) * spacing;
    float startX = (windowSize.x - totalWidth) / 2.f;
    float y = windowSize.y / 2.f - spriteHeight / 2.f;
    for (size_t i = 0; i < deckNames.size(); ++i) {
        sf::Sprite sprite(textureManager.getTexture("../../resources/textures/deckTexture.png"));
    sprite.setPosition(sf::Vector2f(startX + i * (spriteWidth + spacing), y));
    sf::Vector2u texSize = deckTex.getSize();
    sprite.setScale(sf::Vector2f(spriteWidth / float(texSize.x), spriteHeight / float(texSize.y)));
        deckSprites.push_back(sprite);
    }
}

void DeckSelectionScreen::update() {
    // Gestione particelle e animazioni (da HomePage)
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    hoveredDeckIndex = getHoveredDeckIndex(mousePos);
}

void DeckSelectionScreen::draw() {
    // Sfondo
    sf::Sprite bgSprite(backgroundTexture);
    bgSprite.setScale(sf::Vector2f(
        float(windowSize.x) / backgroundTexture.getSize().x,
        float(windowSize.y) / backgroundTexture.getSize().y
    ));
    window.draw(bgSprite);
    // Particelle (da HomePage)
    // Deck sprites
    for (size_t i = 0; i < deckSprites.size(); ++i) {
        if (int(i) == hoveredDeckIndex) {
            deckSprites[i].setColor(sf::Color(200, 200, 255)); // Effetto hover
        } else {
            deckSprites[i].setColor(sf::Color::White);
        }
        window.draw(deckSprites[i]);
    }
    // Puoi aggiungere il nome del deck sotto ogni sprite
    // ...
}

int DeckSelectionScreen::getHoveredDeckIndex(const sf::Vector2i& mousePos) const {
    for (size_t i = 0; i < deckSprites.size(); ++i) {
        if (deckSprites[i].getGlobalBounds().contains(sf::Vector2f(mousePos))) {
            return int(i);proce
        }
    }
    return -1;
}

int DeckSelectionScreen::getSelectedDeckIndex() const {
    return selectedDeckIndex;
}

std::string DeckSelectionScreen::getSelectedDeckName() const {
    if (selectedDeckIndex >= 0 && selectedDeckIndex < int(deckNames.size())) {
        return deckNames[selectedDeckIndex];
    }
    return "";
}

void DeckSelectionScreen::drawSelectedDeckText(const sf::Font& font) {
    if (selectedDeckIndex >= 0 && selectedDeckIndex < int(deckNames.size())) {
        sf::Text selectedText(font, "Hai selezionato il deck: " + deckNames[selectedDeckIndex], 40);
        
        selectedText.setCharacterSize(40);
        selectedText.setFillColor(sf::Color::Yellow);
    selectedText.setPosition(sf::Vector2f(windowSize.x / 2.f - 250.f, windowSize.y * 0.8f));
        window.draw(selectedText);
    }
}