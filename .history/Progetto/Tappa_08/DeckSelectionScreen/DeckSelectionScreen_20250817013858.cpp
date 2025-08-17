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

    // Parametri deckbox
    float boxWidth = 220.f;
    float boxHeight = 260.f;
    float spacing = 60.f;
    int numDecks = 3;
    float totalWidth = numDecks * boxWidth + (numDecks - 1) * spacing;
    float startX = windowSize.x / 2.f - totalWidth / 2.f;
    float boxY = windowSize.y / 2.f - boxHeight / 2.f;

    // Texture diverse per ogni deck
    std::vector<std::string> texPaths = {
        "../../Progetto/resources/textures/deckTex1.png",
        "../../Progetto/resources/textures/deckTex2.png",
        "../../Progetto/resources/textures/deckTex3.png"
    };
    std::vector<std::string> deckLabels = {
        "Deck: Blue-Eyes", "Deck: Dark Magician", "Deck: Red-Eyes"
    };

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    hoveredDeckIndex = -1;

    for (int i = 0; i < numDecks; ++i) {
        float boxX = startX + i * (boxWidth + spacing);
        sf::FloatRect boxRect(sf::Vector2f(boxX, boxY), sf::Vector2f(boxWidth, boxHeight));
        // Mouse over detection
        bool isHovered = boxRect.contains(sf::Vector2f(mousePos));
        if (isHovered) hoveredDeckIndex = i;

        // Rettangolo nero con bordo
        sf::RectangleShape rect(sf::Vector2f(boxWidth, boxHeight));
        rect.setPosition(sf::Vector2f(boxX, boxY));
        rect.setFillColor(sf::Color(20, 20, 20, 220));
        rect.setOutlineColor(sf::Color(180, 180, 180));
        rect.setOutlineThickness(3.f);
        window.draw(rect);

        // Deckbox centrata nel rettangolo
        sf::Texture& deckTex = textureManager.getTexture(texPaths[i]);
        sf::Sprite sprite(deckTex);
        sf::Vector2u texSize = deckTex.getSize();
        float baseScale = std::min((boxWidth-20) / float(texSize.x), (boxHeight-80) / float(texSize.y));
        float spriteScale = baseScale;
        if (isHovered) spriteScale *= 1.18f; // Effetto zoom
        sprite.setScale(sf::Vector2f(spriteScale, spriteScale));
        float spriteX = boxX + (boxWidth - texSize.x * spriteScale) / 2.f;
        float spriteY = boxY + 15.f - (isHovered ? 10.f : 0.f); // Sposta leggermente verso l'alto se zoom
        sprite.setPosition(sf::Vector2f(spriteX, spriteY));
        window.draw(sprite);

        // Testo sotto la deckbox
        sf::Text deckNameText(font, deckLabels[i], 28);
        deckNameText.setFillColor(sf::Color::White);
        deckNameText.setStyle(sf::Text::Bold);
        float textX = boxX + (boxWidth - deckNameText.getLocalBounds().size.x) / 2.f;
        float textY = boxY + boxHeight - 40.f;
        deckNameText.setPosition(sf::Vector2f(textX, textY));
        window.draw(deckNameText);

        // Scritta di selezione deck
        if (selectedDeckIndex == i) {
            sf::Text selectedText(font, "Hai selezionato il deck: " + deckLabels[i], 32);
            selectedText.setFillColor(sf::Color::Yellow);
            selectedText.setStyle(sf::Text::Bold);
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

// ...existing code...

void DeckSelectionScreen::handleClick() {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    float boxWidth = 220.f;
    float boxHeight = 260.f;
    float spacing = 60.f;
    int numDecks = 3;
    float totalWidth = numDecks * boxWidth + (numDecks - 1) * spacing;
    float startX = windowSize.x / 2.f - totalWidth / 2.f;
    float boxY = windowSize.y / 2.f - boxHeight / 2.f;
    for (int i = 0; i < numDecks; ++i) {
        float boxX = startX + i * (boxWidth + spacing);
    sf::FloatRect boxRect(static_cast<float>(boxX), static_cast<float>(boxY), static_cast<float>(boxWidth), static_cast<float>(boxHeight));
    if (boxRect.contains(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))) {
            selectedDeckIndex = i;
            break;
        }
    }
}