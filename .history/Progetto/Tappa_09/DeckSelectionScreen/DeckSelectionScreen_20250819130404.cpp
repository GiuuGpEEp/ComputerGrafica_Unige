#include "DeckSelectionScreen.h"
#include <iostream>

DeckSelectionScreen::DeckSelectionScreen(sf::RenderWindow& window, TextureManager& textureManager, ResourceManager& resourceManager, const sf::Texture& backgroundTexture, sf::Vector2u windowSize, sf::Font& font)
    : textureManager(textureManager), resourceManager(resourceManager), font(font), backgroundSprite(backgroundTexture), titleText(font, "", 10), selectionText(font, "", 34)
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
        "Blue-Eyes", "Dark Magician", "Red-Eyes"
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

    // Titolo schermata
    titleText = sf::Text(font, "Seleziona un Deck", 60);
    titleText.setStyle(sf::Text::Bold);
    titleText.setFillColor(sf::Color(255, 220, 120));
    titleText.setOutlineColor(sf::Color(0,0,0,200));
    titleText.setOutlineThickness(4.f);
    sf::FloatRect tBounds = titleText.getLocalBounds();
    titleText.setOrigin(sf::Vector2f(tBounds.size.x/2.f, tBounds.size.y/2.f));
    titleText.setPosition(sf::Vector2f(windowSize.x/2.f, windowSize.y * 0.18f));

    // Testo di selezione (inizialmente vuoto)
    selectionText.setFillColor(sf::Color(255, 215, 0));
    selectionText.setStyle(sf::Text::Bold);
    selectionText.setOutlineColor(sf::Color(0,0,0,180));
    selectionText.setOutlineThickness(3.f);
}

void DeckSelectionScreen::update(sf::RenderWindow& window) {
    float deltaTime = particleClock.restart().asSeconds();
    // Gestione fade
    if(fadeMode != FadeMode::None){
        fadeTimer += deltaTime;
        float t = std::min(1.f, fadeTimer / std::max(0.0001f, fadeDuration));
        if(fadeMode == FadeMode::In){
            // da nero (255) a visibile (0)
            fadeAlpha = 255.f * (1.f - t);
            if(t >= 1.f){
                fadeMode = FadeMode::None;
                fadeAlpha = 0.f;
                fadeOutCompleted = false;
            }
        } else if(fadeMode == FadeMode::Out){
            // da visibile (0) a nero (255)
            fadeAlpha = 255.f * t;
            if(t >= 1.f){
                fadeMode = FadeMode::None;
                fadeAlpha = 255.f;
                fadeOutCompleted = true;
            }
        }
    }
    // Aggiorna particelle
    for (size_t i = 0; i < particles.size(); ++i) {
        sf::Vector2f pos = particles[i].getPosition();
        pos += particleVel[i] * deltaTime;
        particles[i].setPosition(pos);
        float alpha = particleAlpha[i];
        alpha -= deltaTime * 60.f; // fade out
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

    // Disegna il titolo
    window.draw(titleText);
     
    // Disegna particelle animate
    for (auto& p : particles) window.draw(p);

     sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    for (int i = 0; i < static_cast<int>(deckRects.size()); ++i) {
        window.draw(deckRects[i]);
        
        // Copia locale dello sprite per applicare effetti hover senza modificare l'originale
        sf::Sprite sprite = deckSprites[i];
        const sf::Texture& tex = sprite.getTexture();
        sf::Vector2u texSize = tex.getSize();
        if (texSize.x == 0 || texSize.y == 0) {
            // Texture non ancora valida / caricata male
            continue;
        }

        sf::FloatRect bounds = deckRects[i].getGlobalBounds();
        bool isHovered = bounds.contains(static_cast<sf::Vector2f>(mousePos));

        float boxWidth = deckRects[i].getSize().x;
        float boxHeight = deckRects[i].getSize().y;
        float baseScale = std::min((boxWidth - 20.f) / float(texSize.x), (boxHeight - 80.f) / float(texSize.y));
        if (baseScale <= 0.f || !std::isfinite(baseScale)) baseScale = 1.f; // fallback sicuro
        float spriteScale = isHovered ? baseScale * 1.18f : baseScale;
        float extraYOffset = isHovered ? -10.f : 0.f;

        float boxX = deckRects[i].getPosition().x;
        float boxY = deckRects[i].getPosition().y;
        float spriteX = boxX + (boxWidth - texSize.x * spriteScale) / 2.f;
        float spriteY = boxY + 15.f + extraYOffset;
        sprite.setScale(sf::Vector2f(spriteScale, spriteScale));
        sprite.setPosition(sf::Vector2f(spriteX, spriteY));

        window.draw(sprite);
        window.draw(deckLabelsText[i]);
    }

    // Disegna testo conferma selezione (in basso al centro) se presente
    if(!selectionText.getString().isEmpty()){
        sf::Vector2u ws = window.getSize();
        sf::FloatRect b = selectionText.getLocalBounds();
        selectionText.setOrigin(sf::Vector2f(b.size.x/2.f, b.size.y/2.f));
        selectionText.setPosition(sf::Vector2f(ws.x/2.f, ws.y * 0.9f));
        window.draw(selectionText);
    }

    // Overlay fade (nero sopra, alpha variabile)
    if(fadeMode != FadeMode::None || fadeAlpha > 0.f){
        sf::RectangleShape fadeRect(sf::Vector2f(window.getSize().x, window.getSize().y));
        fadeRect.setFillColor(sf::Color(0,0,0, static_cast<uint8_t>(std::clamp(fadeAlpha, 0.f, 255.f))));
        window.draw(fadeRect);
    }
}

void DeckSelectionScreen::initParticles() {
    particles.clear();
    particleAlpha.clear();
    particleVel.clear();
    particleClock.restart();
    spawnClock.restart();
}

sf::FloatRect DeckSelectionScreen::getDeckBlueEyesBounds() const {
    return deckRects[0].getGlobalBounds();
}

sf::FloatRect DeckSelectionScreen::getDeckDarkMagicianBounds() const {
    return deckRects[1].getGlobalBounds();
}

sf::FloatRect DeckSelectionScreen::getDeckRedEyesBounds() const {
    return deckRects[2].getGlobalBounds();
}

void DeckSelectionScreen::setSelectedDeck(int index){
    selectedDeckIndex = index;
    if(index >=0 && index < static_cast<int>(deckLabelsText.size())){
        // Estraggo solo la parte dopo "Deck: " per nome più pulito
        std::string full = deckLabelsText[index].getString();
        std::string clean = full;
        const std::string prefix = "Deck: ";
        if(full.rfind(prefix,0)==0) clean = full.substr(prefix.size());
        selectionText.setString("Hai selezionato il deck: " + clean);
    }
}

bool DeckSelectionScreen::hasSelection() const {
    return selectedDeckIndex != -1;
}

std::string DeckSelectionScreen::getSelectedDeckName() const {
    if(selectedDeckIndex >=0 && selectedDeckIndex < static_cast<int>(deckLabelsText.size())){
        std::string full = deckLabelsText[selectedDeckIndex].getString();
        const std::string prefix = "Deck: ";
        if(full.rfind(prefix,0)==0) return full.substr(prefix.size());
        return full;
    }
    return "";
}

void DeckSelectionScreen::startFadeIn(float duration){
    fadeDuration = duration;
    fadeTimer = 0.f;
    fadeMode = FadeMode::In;
    fadeAlpha = 255.f; // parte da nero
    fadeOutCompleted = false;
}

void DeckSelectionScreen::startFadeOut(float duration){
    fadeDuration = duration;
    fadeTimer = 0.f;
    fadeMode = FadeMode::Out;
    fadeAlpha = 0.f; // parte da visibile
    fadeOutCompleted = false;
}

bool isFadeOutFinished