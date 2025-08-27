#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "TextureManager/TextureManager.h"

// Configurazione centralizzata per i percorsi delle risorse usati in Tappa_14,
// più un piccolo helper per precaricare le texture più comuni.
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
            static sf::Texture dummy; // non dovrebbe accadere
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
    
    // Metriche UI centralizzate (posizioni e dimensioni testo ricorrenti)
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

        // Metriche di interazione/layout centralizzate
        inline constexpr float HandSpacingPx = 15.f;          // spaziatura tra le carte in mano
        inline constexpr float HandBottomMarginPx = 15.f;     // margine inferiore per la Y della mano del giocatore
        inline constexpr float OpponentHandTopMarginPx = 30.f;// margine superiore per la Y della mano dell'avversario
        inline constexpr float DragThresholdPx = 35.f;        // pixel per avviare il trascinamento
        inline constexpr float CardLiftPx = 50.f;             // offset di sollevamento per la carta selezionata in mano
        inline constexpr float LiftSpeedPxPerSec = 200.f;     // velocità dell'animazione di sollevamento
        inline constexpr float DetailsScrollStepPx = 20.f;    // passo della rotella per il pannello dettagli
        inline constexpr int OverlayHoldMs = 400;             // durata della pressione per aprire overlay (se usato)

        // Aspetto grafico Deck
        inline constexpr float DeckScaleFactor = 0.9f;        // fattore di scala visiva per la pila del deck

        // Metriche griglia overlay DeckSend
        inline constexpr float DeckSendScale = 0.7f;
        inline constexpr float DeckSendPad = 12.f;
        inline constexpr size_t DeckSendCols = 5;

        // Pannello dettagli (carta selezionata / carta pescata) posizione assoluta e dimensioni
        inline constexpr float DetailsPanelPosX = 400.f;
        inline constexpr float DetailsPanelPosY = 150.f;
        inline constexpr float DetailsPanelWidth = 300.f;
        inline constexpr float DetailsPanelHeight = 200.f;

        // Pannello dettagli overlay (Extra/Cimitero) posizione come fattori e dimensioni fisse
        inline constexpr float OverlayDetailsPosXFactor = 0.07f;
        inline constexpr float OverlayDetailsPosYFactor = 0.12f;
        inline constexpr float OverlayDetailsWidth = 340.f;
        inline constexpr float OverlayDetailsHeight = 260.f;
    }

    // Metriche temporali centralizzate
    namespace Timing {
        inline constexpr float FadeInFastSec = 0.5f;
        inline constexpr float FadeOutFastSec = 0.4f;
        inline constexpr float BattleFeedbackSec = 2.0f;
        inline constexpr float NoDeckWarningSec = 3.0f;
        inline constexpr float NoDeckWarningFadeSec = 0.8f;
    }
    

}
