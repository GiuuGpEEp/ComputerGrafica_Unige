#include "SageWithEyesOfBlueActivationEffect.h"
#include <algorithm>

static inline bool _isBEorSpirit(const Card& c){
    if(c.getType() != Type::Monster) return false;
    const std::string &n = c.getName();
    return n == "Drago Bianco Occhi Blu" || n == "Drago Spirito del Bianco" || n == "Drago Spirito Occhi Blu";
}

bool SageWithEyesOfBlueActivationEffect::canActivate(Game& game, int ownerIdx) const{
    // Può attivarsi solo nel Main Phase del proprietario e se controlla almeno 1 mostro (da mandare al GY)
    (void)ownerIdx;
    // Timing: delegato a Game::activateMonsterFromHand, qui richiediamo solo che ci sia almeno un mostro controllato
    // e almeno una fonte disponibile per BEWD/Spirit (mano, deck o cimitero)
    const auto &mz = (ownerIdx==game.getTurn().getCurrentPlayerIndex()) ? game.getMonsterZone() : game.getOpponentMonsterZone();
    if(mz.empty()) return false;
    // Verifica risorsa: Deck o mano o GY contengono un target valido
    const Deck &d = game.getDeckOf(ownerIdx);
    auto inDeck = d.collectWhere(_isBEorSpirit);
    if(!inDeck.empty()) return true;
    const auto &hand = game.getHandOf(ownerIdx);
    if(std::any_of(hand.begin(), hand.end(), _isBEorSpirit)) return true;
    const auto &gy = game.getGraveyardOf(ownerIdx);
    if(std::any_of(gy.begin(), gy.end(), _isBEorSpirit)) return true;
    return false;
}

bool SageWithEyesOfBlueActivationEffect::resolve(Game& game, int ownerIdx){
    // Sequenza:
    // 1) Richiedi selezione di un proprio mostro -> al resolvePendingSelectOwnMonster, l'hook CardSentToGrave prosegue al passo 2
    if(!game.requestSelectOwnMonsterOnField(ownerIdx)) return false;
    // 2) Una volta mandato al GY, prova ad evocare BEWD/Spirit da mano, Deck o Cimitero (in quest'ordine: Deck -> mano -> Cimitero)
    int subId = 0;
    subId = game.events().subscribe(GameEventType::CardSentToGrave, [ownerIdx, &game, &subId](){
        // Evita ri-entrate multiple disiscrivendo subito
        game.events().unsubscribe(subId);
        // Dal Deck
        Deck &d = game.getDeckOf(ownerIdx);
        auto deckCands = d.collectWhere(_isBEorSpirit);
        if(!deckCands.empty()){
            std::string name = deckCands.front().getName();
            auto picked = d.removeFirstByName(name);
            if(picked.has_value()){
                game.mirrorExternalDeckRemoveByName(name, &d);
                // Chiedi posizione con scelta
                (void)game.requestSpecialSummonWithChoice(ownerIdx, std::move(*picked));
                return;
            }
        }
        // Dalla mano
        auto &hand = game.getHandOf(ownerIdx);
        for(size_t i=0;i<hand.size();++i){
            if(_isBEorSpirit(hand[i])){
                Card c = hand[i];
                hand.erase(hand.begin()+i);
                (void)game.requestSpecialSummonWithChoice(ownerIdx, std::move(c));
                return;
            }
        }
        // Dal Cimitero
        auto &gy = game.getGraveyardOf(ownerIdx);
        for(size_t i=0;i<gy.size();++i){
            if(_isBEorSpirit(gy[i])){
                Card c = gy[i];
                gy.erase(gy.begin()+i);
                (void)game.requestSpecialSummonWithChoice(ownerIdx, std::move(c));
                return;
            }
        }
    });
    return true;
}
