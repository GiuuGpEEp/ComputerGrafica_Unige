    #include "HomePage.h"

HomePage::HomePage(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2u& windowSize, sf::Texture& texture, const std::string& deckName)
    : window(window), deckName(deckName), backgroundSprite(texture), menuText(font, "", 36u), deckText(font, "Deck: " + deckName, std::max(22u, static_cast<unsigned int>(windowSize.y * 0.03f))) {
    initParticles();
    // Imposta la scala dello sfondo
    sf::Vector2u textureSize = texture.getSize();
    float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(textureSize.x);
    float scaleY = (static_cast<float>(windowSize.y) / static_cast<float>(textureSize.y)) * 1.20f;
    backgroundSprite.setScale(sf::Vector2f(scaleX, scaleY));

    // Crea e posiziona le voci del menu
    unsigned int fontSize = std::max(36u, static_cast<unsigned int>(windowSize.y * 0.045f));
    float leftMargin = windowSize.x * 0.08f;
    float topMargin = windowSize.y * 0.22f;
    float spacing = windowSize.y * 0.08f;
    menuTexts.clear();
    for (size_t i = 0; i < menuItems.size(); ++i) {
        sf::Text t(font, menuItems[i], fontSize);
        t.setPosition(sf::Vector2f(leftMargin, topMargin + i * spacing));
        t.setStyle(sf::Text::Bold);
        t.setFillColor(sf::Color::White);
        t.setOutlineColor(sf::Color(0,0,0,180));
        t.setOutlineThickness(3.f);
        menuTexts.push_back(t);
    }

    // Scritta nome deck selezionato in alto
    deckText.setFillColor(sf::Color(255, 255, 180));
    deckText.setStyle(sf::Text::Bold);
    deckText.setOutlineColor(sf::Color(120, 80, 0, 180));
    deckText.setOutlineThickness(2.f);
    
    // Centra orizzontalmente, posiziona in alto
    sf::FloatRect deckBounds = deckText.getLocalBounds();
    deckText.setPosition(sf::Vector2f((windowSize.x - deckBounds.size.x) / 2.0f, windowSize.y * 0.04f));

}

//bool HomePage::isMouseOverMenu() {
//    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
//    for (size_t i = 0; i < menuTexts.size(); ++i) {
//        sf::FloatRect bounds = menuTexts[i].getGlobalBounds();
//        if (bounds.contains(static_cast<sf::Vector2f>(mousePos))) {
//            return true;
//        }
//    }
//    return false;
//}

void HomePage::draw(sf::RenderWindow& window) {
    window.draw(backgroundSprite);
    
    // Disegna particelle animate
    for (auto& p : particles) window.draw(p);

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    for (size_t i = 0; i < menuTexts.size(); ++i) {
        sf::FloatRect bounds = menuTexts[i].getGlobalBounds();
        if (bounds.contains(static_cast<sf::Vector2f>(mousePos))) {
            menuTexts[i].setFillColor(sf::Color(255, 180, 60));
            menuTexts[i].setOutlineColor(sf::Color(120, 80, 0));
            menuTexts[i].setOutlineThickness(5.f);
        } else {
            menuTexts[i].setFillColor(sf::Color::White);
            menuTexts[i].setOutlineColor(sf::Color(0,0,0,180));
            menuTexts[i].setOutlineThickness(3.f);
        }
        window.draw(menuTexts[i]);
    }

    window.draw(deckText);
}

sf::FloatRect HomePage::getGiocaBounds() const{
    return menuTexts[0].getGlobalBounds();
}

sf::FloatRect HomePage::getSelectionBounds() const{
    return menuTexts[1].getGlobalBounds();
}

void HomePage::initParticles() {
    particles.clear();
    particleAlpha.clear();
    particleVel.clear();
    particleClock.restart();
    spawnClock.restart();
}

void HomePage::update(){
    
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

void HomePage::setDeckName(const std::string& name) {
    deckName = name;
}