#include "GameWiring.h"
#include "GameLogic/Game/Effects/cards/WhiteStoneLegendEffect.h"
#include "GameLogic/Game/Effects/cards/WhiteStoneAncientsEffect.h"
#include "GameLogic/Game/Effects/cards/ReliquarioDelDragoEffect.h"
#include "GameLogic/Game/Effects/cards/PiantoDiArgentoEffect.h"
#include "ResourceManager/ResourceManager.h"

void registerDefaultEffects(Game &game){
    game.registerEffectForCardName("Pietra Bianca della Leggenda", std::make_unique<WhiteStoneLegendEffect>());
    game.registerEffectForCardName("Pietra Bianca degli Antichi", std::make_unique<WhiteStoneAncientsEffect>());
    // Activation effects for spells
    game.registerActivationForCardName("Reliquario del Drago", std::make_unique<ReliquarioDelDragoEffect>());
    game.registerActivationForCardName("Pianto di Argento", std::make_unique<PiantoDiArgentoEffect>());
}

void createGameAndAttachResources(
    const std::string &selectedDeckName,
    Deck &deck,
    Deck &opponentDeck,
    ResourceManager & /*resourceManager*/,
    std::unique_ptr<Game> &game,
    DrawController &drawController,
    AIController &aiP2,
    std::vector<Card>* &handPtr,
    std::vector<Card> &p1STCards,
    std::vector<Card> &p2STCards,
    std::optional<Card> &p1FieldSpellCard,
    std::optional<Card> &p2FieldSpellCard
){
    // `deck` and `opponentDeck` are expected to be already set by the caller
    Player p1("Player1", deck);
    Player p2("Player2", opponentDeck);
    game = std::make_unique<Game>(p1, p2);
    aiP2.attachGame(game.get());
    // Clear ST/FieldSpell UI caches
    p1STCards.clear();
    p2STCards.clear();
    p1FieldSpellCard.reset();
    p2FieldSpellCard.reset();
    // Register effects and attach controllers/decks
    registerDefaultEffects(*game);
    game->attachExternalDeck(&deck);
    game->attachDrawController(&drawController);
    // Ensure visible hand pointer refers to absolute Player 0
    if(game){
        int cur = game->getTurn().getCurrentPlayerIndex();
        handPtr = (cur==0) ? &game->current().getHand() : &game->opponent().getHand();
        if(handPtr) handPtr->clear();
    }
}
