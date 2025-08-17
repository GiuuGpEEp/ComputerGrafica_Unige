#include "DeckSelectionScreen.h"
#include <iostream>

DeckSelectionScreen::DeckSelectionScreen(sf::RenderWindow& window, TextureManager& textureManager, ResourceManager& resourceManager, const sf::Texture& backgroundTexture, sf::Vector2u windowSize, sf::Font& font)
    : window(window), textureManager(textureManager), resourceManager(resourceManager), backgroundTexture(backgroundTexture), windowSize(windowSize), font(font)
{
    deckNames = resourceManager.getDeckNames();
    // Parametri deckbox
    float boxWidth = 220.f;
    float boxHeight = 260.f;
    float spacing = 60.f;
    int numDecks = 3;
    float totalWidth = numDecks * boxWidth + (numDecks - 1) * spacing;
    float startX = (windowSize.x - totalWidth) / 2.f;
    float boxY = windowSize.y / 2.f - boxHeight / 2.f;

    // Texture e label per ogni deck
    std::vector<std::string> texPaths = {
        "../../Progetto/resources/textures/deckTex1.png",
        "../../Progetto/resources/textures/deckTex2.png",
        "../../Progetto/resources/textures/deckTex3.png"
    };
    std::vector<std::string> deckLabels = {
        "Deck: Blue-Eyes", "Deck: Dark Magician", "Deck: Red-Eyes"
    };
    for (int i = 0; i < numDecks; ++i) {
        float boxX = startX + i * (boxWidth + spacing);
        // Sprite
        sf::Texture& deckTex = textureManager.getTexture(texPaths[i]);
        sf::Sprite sprite(deckTex);
        sf::Vector2u texSize = deckTex.getSize();
        float baseScale = std::min((boxWidth-20) / float(texSize.x), (boxHeight-80) / float(texSize.y));
        sprite.setScale(sf::Vector2f(baseScale, baseScale));
        float spriteX = boxX + (boxWidth - texSize.x * baseScale) / 2.f;
        float spriteY = boxY + 15.f;
        sprite.setPosition(sf::Vector2f(spriteX, spriteY));
        deckSprites.push_back(sprite);
        // Label
        sf::Text label(font, deckLabels[i], 28);
        label.setFillColor(sf::Color::White);
        label.setStyle(sf::Text::Bold);
        float textX = boxX + (boxWidth - label.getLocalBounds().size.x) / 2.f;
        float textY = boxY + boxHeight - 40.f;
        label.setPosition(sf::Vector2f(textX, textY));
        deckLabelsText.push_back(label);
        
        // Box rect
        sf::RectangleShape rect(sf::Vector2f(boxWidth, boxHeight));
        rect.setPosition(sf::Vector2f(boxX, boxY));
        rect.setFillColor(sf::Color(20, 20, 20, 220));
        rect.setOutlineColor(sf::Color(180, 180, 180));
        rect.setOutlineThickness(3.f);
        deckRects.push_back(rect);
    }
}

void DeckSelectionScreen::update() {
    // Gestione particelle e animazioni (da HomePage)
    // Mouse over gestito in draw()
}

void DeckSelectionScreen::draw() {
    // Sfondo
    sf::Sprite bgSprite(backgroundTexture);
    bgSprite.setScale(sf::Vector2f(
        float(windowSize.x) / backgroundTexture.getSize().x,
        float(windowSize.y) / backgroundTexture.getSize().y
    ));
    window.draw(bgSprite);

    for (int i = 0; i < deckRects.size(); ++i) {
        window.draw(deckRects[i]);
        // Effetto zoom su hover
        bool isHovered = (hoveredDeckIndex == i);
        sf::Sprite sprite = deckSprites[i];
        if (isHovered) {
            sf::Texture deckTex = sprite.getTexture();
            sf::Vector2u texSize = deckTex.getSize();
            float boxWidth = deckRects[i].getSize().x;
            float boxHeight = deckRects[i].getSize().y;
            float baseScale = std::min((boxWidth-20) / float(texSize.x), (boxHeight-80) / float(texSize.y));
            float spriteScale = baseScale * 1.18f;
            float boxX = deckRects[i].getPosition().x;
            float boxY = deckRects[i].getPosition().y;
            float spriteX = boxX + (boxWidth - texSize.x * spriteScale) / 2.f;
            float spriteY = boxY + 15.f - 10.f;
            sprite.setScale(sf::Vector2f(spriteScale, spriteScale));
            sprite.setPosition(sf::Vector2f(spriteX, spriteY));
        }
        window.draw(sprite);
        window.draw(deckLabelsText[i]);
        // Scritta di selezione deck
        if (selectedDeckIndex == i) {
            sf::Text selectedText(font, "Hai selezionato il deck: " + deckLabelsText[i].getString(), 32);
            selectedText.setFillColor(sf::Color::Yellow);
            selectedText.setStyle(sf::Text::Bold);
            float boxX = deckRects[i].getPosition().x;
            float boxY = deckRects[i].getPosition().y;
            float boxWidth = deckRects[i].getSize().x;
            float boxHeight = deckRects[i].getSize().y;
            float selTextX = boxX + (boxWidth - selectedText.getLocalBounds().size.x) / 2.f;
            float selTextY = boxY + boxHeight + 10.f;
            selectedText.setPosition(sf::Vector2f(selTextX, selTextY));
            window.draw(selectedText);
        }
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

void DeckSelectionScreen::setSelectedDeckIndex(int idx) {
    selectedDeckIndex = idx;
}

