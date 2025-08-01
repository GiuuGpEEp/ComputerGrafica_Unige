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
   for (int i = 0; i < 3; ++i) {
        slots.emplace_back(sf::Vector2f(xStart + i * (slotWidth + spacing), bottomY), monsterTex, Slot::Type::Monster);
        slots.emplace_back(sf::Vector2f(xStart + i * (slotWidth + spacing), bottomY + slotHeight + spacing), spellTex, Slot::Type::SpellTrap);
    }

    slots.emplace_back(sf::Vector2f(600.f, bottomY), deckTex, Slot::Type::Deck);
    slots.emplace_back(sf::Vector2f(600.f, bottomY + slotHeight + spacing), graveTex, Slot::Type::Graveyard);
    slots.emplace_back(sf::Vector2f(680.f, bottomY), extraTex, Slot::Type::Extra);
    slots.emplace_back(sf::Vector2f(680.f, bottomY + slotHeight + spacing), fieldSlotTex, Slot::Type::Field);

}

//Continuare con il draw
 