#include "DeckSelectionScreen.h"
#include <iostream>

DeckSelectionScreen::DeckSelectionScreen(sf::RenderWindow& window, TextureManager& textureManager, ResourceManager& resourceManager, const sf::Texture& backgroundTexture, sf::Vector2u windowSize, sf::Font& font)
    : window(window), textureManager(textureManager), resourceManager(resourceManager), backgroundTexture(backgroundTexture), windowSize(windowSize), font(font)
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
        sf::Texture& deckTex = textureManager.getTexture("../../Progetto/resources/textures/deckTex.png");
        sf::Sprite sprite(deckTex);
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

    // Parametri rettangolo
    float boxWidth = 220.f;
    float boxHeight = 260.f;
    float boxX = windowSize.x / 2.f - boxWidth / 2.f;
    float boxY = windowSize.y / 2.f - boxHeight / 2.f;

    // Rettangolo nero con bordo
    sf::RectangleShape rect(sf::Vector2f(boxWidth, boxHeight));
    rect.setPosition(sf::Vector2f(boxX, boxY));
    rect.setFillColor(sf::Color(20, 20, 20, 220));
    rect.setOutlineColor(sf::Color(180, 180, 180));
    rect.setOutlineThickness(3.f);
    window.draw(rect);

    // Deckbox centrata nel rettangolo
    if (!deckSprites.empty()) {
        sf::Sprite sprite = deckSprites[0];
        sf::Texture deckTex = sprite.getTexture();
        sf::Vector2u texSize = deckTex.getSize();
        float spriteScale = std::min((boxWidth-20) / float(texSize.x), (boxHeight-80) / float(texSize.y));
        sprite.setScale(sf::Vector2f(spriteScale, spriteScale));
        float spriteX = boxX + (boxWidth - texSize.x * spriteScale) / 2.f;
        float spriteY = boxY + 15.f;
        sprite.setPosition(sf::Vector2f(spriteX, spriteY));
        window.draw(sprite);

        // Testo sotto la deckbox
        sf::Text deckNameText(font, "DECK", 28);
        deckNameText.setFont(font); // Assicurati che il font sia disponibile
        deckNameText.setString(deckNames.empty() ? "DECK" : deckNames[0]);
        deckNameText.setCharacterSize(28);
        deckNameText.setFillColor(sf::Color::White);
        deckNameText.setStyle(sf::Text::Bold);
        float textX = windowSize.x / 2.f - deckNameText.getLocalBounds(). / 2.f;
        float textY = boxY + boxHeight - 40.f;
        deckNameText.setPosition(sf::Vector2f(textX, textY));
        window.draw(deckNameText);
    }
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