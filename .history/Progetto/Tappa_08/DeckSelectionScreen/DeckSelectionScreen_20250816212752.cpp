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
        sf::Sprite sprite(te);
        // Usa una texture rappresentativa del deck, qui esempio generico
        sprite.setTexture(textureManager.getTexture("../resources/textures/deckTexture.png"));
        sprite.setPosition(startX + i * (spriteWidth + spacing), y);
        sprite.setScale(spriteWidth / sprite.getTexture()->getSize().x, spriteHeight / sprite.getTexture()->getSize().y);
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
    bgSprite.setScale(
        float(windowSize.x) / backgroundTexture.getSize().x,
        float(windowSize.y) / backgroundTexture.getSize().y
    );
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
            return int(i);
        }
    }
    return -1;
}

int DeckSelectionScreen::getSelectedDeckIndex() const {
    return selectedDeckIndex;
}

void DeckSelectionScreen::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        int idx = getHoveredDeckIndex(sf::Mouse::getPosition(window));
        if (idx != -1) {
            selectedDeckIndex = idx;
            std::cout << "Deck selezionato: " << deckNames[idx] << std::endl;
        }
    }
}

std::string DeckSelectionScreen::getSelectedDeckName() const {
    if (selectedDeckIndex >= 0 && selectedDeckIndex < int(deckNames.size())) {
        return deckNames[selectedDeckIndex];
    }
    return "";
}

void DeckSelectionScreen::drawSelectedDeckText(const sf::Font& font) {
    if (selectedDeckIndex >= 0 && selectedDeckIndex < int(deckNames.size())) {
        sf::Text selectedText;
        selectedText.setFont(font);
        selectedText.setString("Hai selezionato il deck: " + deckNames[selectedDeckIndex]);
        selectedText.setCharacterSize(40);
        selectedText.setFillColor(sf::Color::Yellow);
        selectedText.setPosition(windowSize.x / 2.f - 250.f, windowSize.y * 0.8f);
        window.draw(selectedText);
    }
}