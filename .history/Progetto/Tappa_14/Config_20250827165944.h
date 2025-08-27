#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "TextureManager/TextureManager.h"

// Centralized configuration for resource paths used in Tappa_14,
// plus a tiny helper to preload commonly used textures.
namespace AppConfig {
    const std::string resourceBase = "../../Progetto/resources/";
    const std::string texturePath  = resourceBase + "textures/";
    const std::string fontPath     = texturePath + "ITCKabelStdDemi.TTF";
    const std::string cardsJsonPath = resourceBase + "jsonData/cards.json";
    const std::string decksJsonPath = resourceBase + "jsonData/decks.json";
     
    enum class TextureKey {
        FieldBackground,
        MonsterText,
        SpellTrap,
        Deck,
        Graveyard,
        ExtraDeck,
        FieldSpell,
        CardBack,
        CardFrontNotSet,
        StartScreen,
        HomeScreen,
        DeckSelectionScreen
    };

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

    inline const std::unordered_map<TextureKey, std::string>& texturePathMap(){
        static const std::unordered_map<TextureKey, std::string> m = {
            {TextureKey::FieldBackground,     texturePath + "backgroundTexture.png"},
            {TextureKey::MonsterText,         texturePath + "monsterText.png"},
            {TextureKey::SpellTrap,           texturePath + "SpellTrapTexture.png"},
            {TextureKey::Deck,                texturePath + "deckTexture.png"},
            {TextureKey::Graveyard,           texturePath + "graveTexture.png"},
            {TextureKey::ExtraDeck,           texturePath + "ExtraDeckTexture.png"},
            {TextureKey::FieldSpell,          texturePath + "fieldSpell.png"},
            {TextureKey::CardBack,            texturePath + "Texture1.png"},
            {TextureKey::CardFrontNotSet,     texturePath + "CardNotSet.jpg"},
            {TextureKey::StartScreen,         texturePath + "startScreenTexture.png"},
            {TextureKey::HomeScreen,          texturePath + "homeScreenTexture.png"},
            {TextureKey::DeckSelectionScreen, texturePath + "deckSelectionScreenTexture.png"}
        };
        return m;
    }

    inline std::unordered_map<TextureKey, sf::Texture*> buildTextureMap(TextureManager& tm){
        std::unordered_map<TextureKey, sf::Texture*> out;
        for(const auto& kv : texturePathMap()){
            out[kv.first] = &tm.getTexture(kv.second);
        }
        return out;
    }

    inline sf::Texture& findTextureInMap(const std::unordered_map<TextureKey, sf::Texture*> &map, TextureKey key){
        auto it = map.find(key);
        if(it == map.end()){
            static sf::Texture dummy; // should not happen
            std::cerr << "[Config] TextureKey non trovato" << std::endl;
            return dummy;
        }
        return *it->second;
    }

    inline void loadDetailFont(sf::Font& font, const std::string& primaryPath){
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
    
    // UI metrics centralizzate (posizioni e dimensioni testo ricorrenti)
    namespace Ui {
        inline constexpr float PadLeft = 15.f;
        inline constexpr float TopY = 10.f;
        inline constexpr float WarnY = 38.f;
        inline constexpr float TribY = 66.f;
        inline constexpr float SSChoiceY = 94.f;
        inline constexpr float STChoiceY = 122.f;
        inline constexpr float CenterFeedbackY = 70.f;
        inline constexpr unsigned HudSize = 22;
        inline constexpr unsigned SmallSize = 20;

    // Centralized interaction/layout metrics
    inline constexpr float HandSpacingPx = 15.f;          // spacing between cards in hand
    inline constexpr float HandBottomMarginPx = 15.f;     // bottom margin for player hand Y
    inline constexpr float OpponentHandTopMarginPx = 30.f;// top margin for opponent hand Y
    inline constexpr float DragThresholdPx = 35.f;        // pixels to start dragging
    inline constexpr float CardLiftPx = 50.f;             // lifted offset for selected hand card
    inline constexpr float LiftSpeedPxPerSec = 200.f;     // lift animation speed
    inline constexpr float DetailsScrollStepPx = 20.f;    // mouse wheel step for details panel
    inline constexpr int OverlayHoldMs = 400;             // hold duration to open overlays (if used)
    }
    

}
