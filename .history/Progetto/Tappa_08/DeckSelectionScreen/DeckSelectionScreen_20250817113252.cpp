#include "DeckSelectionScreen.h"
#include <iostream>

DeckSelectionScreen::DeckSelectionScreen(sf::RenderWindow& window, TextureManager& textureManager, ResourceManager& resourceManager, const sf::Texture& backgroundTexture, sf::Vector2u windowSize, sf::Font& font)
    : textureManager(textureManager), resourceManager(resourceManager), font(font), backgroundSprite(backgroundTexture)
{
    deckNames = resourceManager.getDeckNames();
    
    initParticles();

    //Inizializzazione dello sfondo
    backgroundSprite.setScale(sf::Vector2f(
        float(window.getSize().x) / backgroundTexture.getSize().x,
        float(window.getSize().y) / backgroundTexture.getSize().y
    ));


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

void DeckSelectionScreen::update(sf::RenderWindow& window) {
    float dt = particleClock.restart().asSeconds();
    // Aggiorna particelle
    for (size_t i = 0; i < particles.size(); ++i) {
        sf::Vector2f pos = particles[i].getPosition();
        pos += particleVel[i] * dt;
        particles[i].setPosition(pos);
        float alpha = particleAlpha[i];
        alpha -= dt * 60.f; // fade out
        if (alpha < 0) alpha = 0;
        particleAlpha[i] = alpha;
        sf::Color c = particles[i].getFillColor();
        c.a = static_cast<unsigned char>(alpha);
        particles[i].setFillColor(c);
    }
    // Rimuovi particelle morte
    for (int i = (int)particles.size()-1; i >= 0; --i) {
        if (particleAlpha[i] <= 0) {
            particles.erase(particles.begin()+i);
            particleAlpha.erase(particleAlpha.begin()+i);
            particleVel.erase(particleVel.begin()+i);
        }
    }
    // Spawna nuove particelle
    sf::Vector2u windowSize = window.getSize();
    if (spawnClock.getElapsedTime().asSeconds() > 0.08f) {
        spawnClock.restart();
        int nNew = rand()%3+1;
        for (int i=0; i<nNew; ++i) {
            float x = static_cast<float>(rand()%windowSize.x);
            float y = static_cast<float>(rand()%windowSize.y);
            float r = 1.5f + static_cast<float>(rand()%20)/10.f;
            sf::CircleShape p(r);
            p.setPosition(sf::Vector2f(x, y));
            p.setFillColor(sf::Color(255, 140, 40, 255));
            particles.push_back(p);
            particleAlpha.push_back(255.f);
            float vx = -20.f + static_cast<float>(rand()%41);
            float vy = -20.f + static_cast<float>(rand()%41);
            particleVel.push_back(sf::Vector2f(vx, vy));
        }
    }
}

void DeckSelectionScreen::draw(sf::RenderWindow& window) {
    // Disegna lo sfondo
    window.draw(backgroundSprite);
     
    // Disegna particelle animate
    for (auto& p : particles) window.draw(p);

    for (int i = 0; i < deckRects.size(); ++i) {
        window.draw(deckRects[i]);
        
        // Effetto zoom su hover
        //bool isHovered = (hoveredDeckIndex == i);
        sf::Sprite sprite = deckSprites[i];
        //if (isHovered) {
        //    sf::Texture deckTex = sprite.getTexture();
        //    sf::Vector2u texSize = deckTex.getSize();
        //    float boxWidth = deckRects[i].getSize().x;
        //    float boxHeight = deckRects[i].getSize().y;
        //    float baseScale = std::min((boxWidth-20) / float(texSize.x), (boxHeight-80) / float(texSize.y));
        //    float spriteScale = baseScale * 1.18f;
        //    float boxX = deckRects[i].getPosition().x;
        //    float boxY = deckRects[i].getPosition().y;
        //    float spriteX = boxX + (boxWidth - texSize.x * spriteScale) / 2.f;
        //    float spriteY = boxY + 15.f - 10.f;
        //    sprite.setScale(sf::Vector2f(spriteScale, spriteScale));
        //    sprite.setPosition(sf::Vector2f(spriteX, spriteY));
        //}
        window.draw(sprite);
        window.draw(deckLabelsText[i]);

        // Scritta di selezione deck
        //if (selectedDeckIndex == i) {
        //    sf::Text selectedText(font, "Hai selezionato il deck: " + deckLabelsText[i].getString(), 32);
        //    selectedText.setFillColor(sf::Color::Yellow);
        //    selectedText.setStyle(sf::Text::Bold);
        //    float boxX = deckRects[i].getPosition().x;
        //    float boxY = deckRects[i].getPosition().y;
        //    float boxWidth = deckRects[i].getSize().x;
        //    float boxHeight = deckRects[i].getSize().y;
        //    float selTextX = boxX + (boxWidth - selectedText.getLocalBounds().size.x) / 2.f;
        //    float selTextY = boxY + boxHeight + 10.f;
        //    selectedText.setPosition(sf::Vector2f(selTextX, selTextY));
        //    window.draw(selectedText);
        //}
    }
}

void DeckSelectionScreen::initParticles() {
    particles.clear();
    particleAlpha.clear();
    particleVel.clear();
    particleClock.restart();
    spawnClock.restart();
}

sf::

