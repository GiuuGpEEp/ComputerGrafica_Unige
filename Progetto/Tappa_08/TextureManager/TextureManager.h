#pragma once
#include <SFML/Graphics.hpp>

class TextureManager{
    public:
        sf::Texture& getTexture(const std::string& filename);
        sf::Texture& getDeck1Texture(const std::string& filename);
    private:
        //Mappa che associa il nome del file (chiave) alla texture (valore)
        std::unordered_map<std::string, sf::Texture> textures;
};