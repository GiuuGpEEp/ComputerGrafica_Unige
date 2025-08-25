#include "WhiteStoneLegendEffect.h"

void WhiteStoneLegendEffect::onEvent(GameEventType type, Game& game){
    if(type != GameEventType::CardSentToGrave) return;
    auto last = game.getLastSentToGrave();
    if(!last.has_value()) return;
    // Nome esatto della carta effetto
    if(last->name != std::string("Pietra Bianca della Leggenda")) return;
    int owner = last->ownerIdx;
    // Cerca nel deck del proprietario un "Drago Bianco Occhi Blu" e aggiungilo alla mano
    // Per semplicità, usiamo l'externalDeck solo per rendering; qui modifichiamo il Deck logico del Player
    Deck &deck = const_cast<Deck&>(game.getDeckOf(owner));
    auto found = deck.removeFirstByName("Drago Bianco Occhi Blu");
    if(found.has_value()){
        game.addToHand(owner, std::move(*found));
    }
    // Non azzeriamo lastSentToGrave per consentire altri effetti che reagiscono allo stesso trigger in catena
}
