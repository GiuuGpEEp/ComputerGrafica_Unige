#pragma once

#include <string>

// Centralized configuration for resource paths used in Tappa_14.
// Minimal, header-only to avoid linkage issues; relies on C++17 inline variables.
namespace AppConfig {
    inline const std::string resourceBase = "../../Progetto/resources/";
    inline const std::string texturePath  = resourceBase + "textures/";
    inline const std::string fontPath     = texturePath + "ITCKabelStdDemi.TTF";
    inline const std::string cardsJsonPath = resourceBase + "jsonData/cards.json";
    inline const std::string decksJsonPath = resourceBase + "jsonData/decks.json";
}
