#include "TextureManager.h"
#include <iostream>

sf::Texture& TextureManager::getTexture(const std::string& filename) {
    // Usa il percorso completo fornito da filename
    auto it = textures.find(filename);
    if (it != textures.end()) {
        return it->second;
    }

    sf::Texture texture;
    if (!texture.loadFromFile(filename)) {
        std::cerr << "ERRORE: Impossibile caricare la texture: " << filename << std::endl;
    }
    textures[filename] = std::move(texture);
    return textures[filename];
}


sf::Texture& TextureManager::getDeck1Texture(const std::string& filename) {
    // Usa il percorso completo fornito da filename
    auto it = textures.find(filename);
    if (it != textures.end()) {
        return it->second;
    }

    sf::Texture texture;
    if (!texture.loadFromFile(filename)) {
        std::cerr << "ERRORE: Impossibile caricare la texture: " << filename << std::endl;
    }
    textures[filename] = std::move(texture);
    return textures[filename];
}