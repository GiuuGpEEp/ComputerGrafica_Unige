#include "WhiteStoneAncientsEffect.h"
#include "../../Game.h"
#include <iostream>

void WhiteStoneAncientsEffect::onEvent(GameEventType type, Game& game){
    if(type != GameEventType::MonsterDestroyed) return;
    auto last = game.getLastSentToGrave();
    if(!last.has_value()) return;
    if(last->name != std::string("Pietra Bianca degli Antichi")) return;
    int owner = last->ownerIdx;
    std::cout << "[Effect] Trigger: Pietra Bianca degli Antichi -> Special Summon BEWD from Deck for player " << owner << std::endl;
    // Cerca nel deck del proprietario e special summon in attacco scoperto
    Deck &deck = game.getDeckOf(owner);
    auto found = deck.removeFirstByName("Drago Bianco Occhi Blu");
    if(!found.has_value()){
        std::cout << "[Effect] BEWD not found in deck" << std::endl;
        return;
    }
    if(!game.specialSummonToMonsterZone(owner, std::move(*found), /*defense*/false, /*faceDown*/false)){
        std::cout << "[Effect] Special Summon failed (zone full?)" << std::endl;
    }
}
