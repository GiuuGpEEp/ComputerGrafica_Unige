#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "TextureManager/TextureManager.h"
#include <SFML/Graphics/Font.hpp>

// Centralized configuration for resource paths used in Stage_14,
// plus a tiny helper to preload commonly used textures.
class AppConfig {
public:
    ~AppConfig() = default;

    // Resource paths
    inline static const std::string resourceBase = "../../Progetto/resources/";
    inline static const std::string texturePath  = resourceBase + "textures/";
    inline static const std::string fontPath     = texturePath + "ITCKabelStdDemi.TTF";
    inline static const std::string cardsJsonPath = resourceBase + "jsonData/cards.json";
    inline static const std::string decksJsonPath = resourceBase + "jsonData/decks.json";

    inline static const std::vector<std::string> textureFiles = {
        texturePath + "backgroundTexture.png",
        texturePath + "monsterText.png",
        texturePath + "SpellTrapTexture.png",
        texturePath + "deckTexture.png",
        texturePath + "graveTexture.png",
        texturePath + "ExtraDeckTexture.png",
        texturePath + "fieldSpell.png",
        texturePath + "Texture1.png",
        texturePath + "CardNotSet.jpg",
        texturePath + "startScreenTexture.png",
        texturePath + "homeScreenTexture.png",
        texturePath + "deckSelectionScreenTexture.png"
    };

    
    static void loadAllTextures(TextureManager& tm){
        for(const auto& path : textureFiles){
            tm.getTexture(path);
        }
    }

    static void loadDetailFont(sf::Font& font, const std::string& primaryPath){
        // use SFML's loadFromFile and provide English messages to avoid spellchecker warnings
        if (!font.loadFromFile(primaryPath)) {
            std::cerr << "ERROR: Unable to load font: " << primaryPath << std::endl;
            if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
                std::cerr << "ERROR: Unable to load any font!" << std::endl;
            } else {
                std::cout << "Loaded system font: arial.ttf" << std::endl;
            }
        } else {
            std::cout << "Loaded font: " << primaryPath << std::endl;
        }
    }

};
