#include "SageWithEyesOfBlueEffect.h"
#include <algorithm>
#include <iostream>

static inline bool isBlueEyesMonster(const Card& c){
    if(c.getType() != Type::Monster) return false;
    const std::string &nm = c.getName();
    // Semplice check sul nome contiene "Occhi di Blu"; estendibile a blacklist/whitelist
    return nm.find("Occhi di Blu") != std::string::npos;
}

void SageWithEyesOfBlueEffect::onEvent(GameEventType type, Game& game){
    if(type != GameEventType::NormalSummon) return;
    // Verifica se l'ultima Normal Summon è proprio "Saggio con Occhi di Blu"
    auto lastNS = game.getLastNormalSummon();
    if(!lastNS.has_value()) return;
    if(lastNS->name != std::string("Saggio con Occhi di Blu")) return;
    int owner = lastNS->ownerIdx;
    // Richiede: mandare 1 mostro che controlli al Cimitero
    auto &mz = (owner==0 ? game.getMonsterZone() : game.getOpponentMonsterZone());
    if(mz.empty()) return; // nessun mostro da mandare
    // Semplificazione: manda l'ultimo mostro controllato (nessuna UI di scelta ancora)
    size_t idxToSend = mz.size()-1;
    // Implementiamo una rimozione sicura: usa API interne per distruggere quel mostro
    // Poiché non abbiamo un helper generico, usiamo un giro: infliggi 0 e distruggi manualmente.
    // Qui invece replico la logica di tributo per spostarlo a GY mantenendo flags
    int cur = game.getTurn().getCurrentPlayerIndex();
    int zoneOwner = owner == cur ? cur : (1-cur);
    // Ricostruisci un vettore con l'indice reale nella zona del proprietario assoluto
    // Nota: getMonsterZone() e getOpponentMonsterZone() sono rispetto al giocatore di turno.
    size_t realIdx = idxToSend;
    // Esegui una rimozione dal campo verso il GY del proprietario assoluto
    // Copia locale della carta prima della rimozione
    // Accesso diretto agli stati non è esposto; sfruttiamo moveCard solo per carte nella mano.
    // Workaround: tributiamo come se fosse effetto (riusa tributeMonsters su owner corrente se coincide)
    if(owner == cur){
        (void)game.tributeMonsters(std::vector<size_t>{realIdx});
    } else {
        // Se non è il turno del proprietario, non possiamo usare le API zone-current; abort per ora
        std::cout << "[SageWithEyesOfBlue] Effetto on-summon: impossibile mandare mostro al GY (owner non-current)" << std::endl;
        return;
    }
    // Poi aggiungi 1 mostro "Occhi di Blu" dal Deck alla mano
    Deck &d = game.getDeckOf(owner);
    auto candidates = d.collectWhere(isBlueEyesMonster);
    if(candidates.empty()) return;
    // Semplificazione: aggiungi il primo
    const std::string pickName = candidates.front().getName();
    auto picked = d.removeFirstByName(pickName);
    if(!picked.has_value()) return;
    game.mirrorExternalDeckRemoveByName(pickName, &d);
    game.addToHand(owner, std::move(*picked));
}
