#include "WhiteStoneLegendEffect.h"
#include <iostream>

void WhiteStoneLegendEffect::onEvent(GameEventType type, Game& game){
    if(type != GameEventType::CardSentToGrave) return;
    std::cout << "[Effect] Dispatch CardSentToGrave -> WhiteStoneLegendEffect check" << std::endl;
    auto last = game.getLastSentToGrave();
    if(!last.has_value()) return;
    // Nome esatto della carta effetto
    std::cout << "[Effect] Last sent: '" << last->name << "' ownerIdx=" << last->ownerIdx << std::endl;
    if(last->name != std::string("Pietra Bianca della Leggenda")) return;
    std::cout << "[Effect] Trigger: Pietra Bianca della Leggenda -> search BEWD" << std::endl;
    int owner = last->ownerIdx;
    // Cerca nel deck del proprietario un "Drago Bianco Occhi Blu" e aggiungilo alla mano
    // Per semplicità, usiamo l'externalDeck solo per rendering; qui modifichiamo il Deck logico del Player
    Deck &deck = const_cast<Deck&>(game.getDeckOf(owner));
    auto found = deck.removeFirstByName("Drago Bianco Occhi Blu");
    if(!found.has_value()){
        std::cout << "[Effect] BEWD not found in owner deck" << std::endl;
    }
    if(found.has_value()){
        // Mantieni in sync anche il Deck esterno UI
        game.mirrorExternalDeckRemoveByName("Drago Bianco Occhi Blu");
        game.addToHand(owner, std::move(*found));
        std::cout << "[Effect] Added 'Drago Bianco Occhi Blu' to hand of player " << owner << std::endl;
    }
    
}
