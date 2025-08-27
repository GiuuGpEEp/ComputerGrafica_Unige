#pragma once

#include <string>
#include <vector>
#include "TextureManager/TextureManager.h"

// Centralized configuration for resource paths used in Tappa_14,
// plus a tiny helper to preload commonly used textures.
namespace AppConfig {
    inline const std::string resourceBase = "../../Progetto/resources/";
    inline const std::string texturePath  = resourceBase + "textures/";
    inline const std::string fontPath     = texturePath + "ITCKabelStdDemi.TTF";
    inline const std::string cardsJsonPath = resourceBase + "jsonData/cards.json";
    inline const std::string decksJsonPath = resourceBase + "jsonData/decks.json";

    inline const std::vector<std::string> textureFiles = {
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

    inline void loadAllTextures(TextureManager& tm){
        for(const auto& path : textureFiles){
            tm.getTexture(path);
        }
    }

    void loadDetailFont(sf::Font& font, const std::string& primaryPath){
        if (!font.openFromFile(primaryPath)) {
            std::cerr << "ERRORE: Impossibile caricare font: " << primaryPath << std::endl;
            if (!font.openFromFile("C:/Windows/Fonts/calibri.ttf")) {
                std::cerr << "ERRORE: Impossibile caricare nessun font!" << std::endl;
            } else {
                std::cout << "Caricato font di sistema: calibri.ttf" << std::endl;
            }
        } else {
            std::cout << "Caricato font: " << primaryPath << std::endl;
        }
    }
    
}
