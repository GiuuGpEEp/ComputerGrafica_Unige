#include "Field.h"

Field::Field(sf::Texture& backgroundTexture) 
    : background(backgroundTexture) {
    // Imposta lo sprite di sfondo con la texture fornita
    background.setTexture(backgroundTexture);
}
