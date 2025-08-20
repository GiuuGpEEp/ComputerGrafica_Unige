#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

class TextureManager{
    public:
        // Restituisce (e cache) una texture dato il percorso completo
        sf::Texture& getTexture(const std::string& filename);
    private:
        // Mappa che associa il percorso completo (chiave) alla texture (valore)
        std::unordered_map<std::string, sf::Texture> textures;
};