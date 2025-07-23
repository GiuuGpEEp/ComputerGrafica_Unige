#include "Field.h"

Field::Field(
            const sf::Texture& fieldTexture, 
            const sf::Texture& monsterTexture,
            const sf::Texture& spellTexture,
            const sf::Texture& deckTexture,
            const sf::Texture& graveTexture,
            const sf::Texture& extraTexture
        ) : background(fieldTexture) 
{
    background.setScale(sf::Vector2f(
        800.f / background.getTextureRect().size.x,
        600.f / background.getTextureRect().size.y
    ));

    //Recupero le dimensioni di un singolo slot
    const float slotWidth = monsterTexture.getSize().x; //In questo caso prendo uso getSize e non getTextureRect perche' sto prendendo la dimensionde direttamente dalla texture, mentre prima dallo sprite
    const float slotHeight = monsterTexture.getSize().y;

    //stabilisco le coordinate di partenza
    float topY = 100.f;
    float bottomY = 350.f;
    float xStart = 100.f;
    float spacing = 10.f; 

    // 3 mostri sopra, 3 magie sotto per ciascun giocatore
    // 
   

}

//Continuare con il draw
 