#pragma once
#include "../Player/Player.h"
#include "../TurnManager/TurnManager.h"
#include <array>
#include <optional>
#include "../Events/EventDispatcher.h"
#include <functional>
#include <unordered_map>
#include <memory>
#include "Effects/EffectSystem.h"

// Dichiarazioni forward per evitare dipendenze circolari
class DrawController;
class ICardEffect;

enum class CardZone{ 
    Deck, 
    Hand, 
    MonsterZone, 
    SpellTrapZone,
    FieldSpellZone,
    Graveyard,
    Extra,
    Banished
};

class Game {
public:
    Game(const Player& p1, const Player& p2);
    void start();
    bool isStarted() const;

    Player& current();
    Player& opponent();
    TurnManager& getTurn();
    EventDispatcher& events();
    void advancePhase();
    void endTurn();
    void startTurn(); // nuovo inizio turno centralizzato
    int getStartingPlayerIndex() const { return startingPlayerIndex; }
    bool isGameOver() const { return gameOver; }
    std::optional<int> getWinnerIndex() const { return winnerIndex; }

    // Collegamento controller animazioni pescata (gestione automatica pescate Draw Phase + mano iniziale)
    void attachDrawController(DrawController* controller);
    // Collega il Deck usato nel layer UI (per allineare il controllo del deck-out)
    // Ora supporta un deck esterno per ciascun giocatore: fornire playerIdx (0 o 1).
    void attachExternalDeck(Deck* deckPtr, int playerIdx = 0);

    // Gestione limite mano / scarto
    void setHandLimit(size_t limit); // imposta limite per auto discard
    void setDiscardCallback(std::function<void(std::vector<Card>&&)> cb); // callback per avviare animazione scarto nel layer rendering
    void fastForwardToEndPhase(); // avanza direttamente a End Phase applicando regole
    void onDiscardAnimationFinished(); // chiamato dal main quando animazione scarti termina
    bool shouldAutoEndTurn() const; // true se End Phase senza scarti e turno da chiudere

    // Movimento generico carte (per ora solo del giocatore corrente)
    // Ritorna true se spostamento riuscito
    bool moveCard(CardZone from, CardZone to, size_t indexFrom);
    // Helper per banish: rimuove una carta da una zona e la mette nella zona Banished del giocatore corrente
    bool banishFrom(CardZone from, size_t indexFrom);
    // Tribute logic
    int requiredTributesFor(const Card& c) const; // 0/1/2 in base al livello
    // Indici dei mostri da tributare (rispetto alla monster zone corrente). Ritorna true se ok.
    bool tributeMonsters(const std::vector<size_t>& zoneIndices);
    // Avvia processo di Normal Summon che richiede tributi: salva indice carta in mano e attende scelta
    bool beginNormalSummonWithTributes(size_t handIndex);
    bool beginNormalSummonWithTributes(size_t handIndex, bool asSet); // overload per distinguere Set
    // Completa la summon dopo che l'UI ha fornito gli indici dei tributi; ritorna true se successo
    bool completePendingNormalSummon(const std::vector<size_t>& tributeIndices);
    bool hasPendingNormalSummon() const { return pendingSummonHandIndex.has_value(); }
    void cancelPendingNormalSummon();
    int getPendingTributesNeeded() const { return pendingSummonHandIndex.has_value() ? pendingTributesNeeded : 0; }

    // Normal Summon tracking
    bool canNormalSummon() const;
    bool tryNormalSummon(size_t handIndex); // per ora solo aggiunge a zona mostri logica
    // Slot-aware Normal Summon/Set: posiziona esplicitamente nello slot [0..MONSTER_ZONE_SIZE-1]
    bool tryNormalSummonToSlot(size_t handIndex, size_t slotIndex);
    void resetNormalSummon();
    const std::vector<Card>& getMonsterZone() const; // accesso read-only
    const std::vector<Card>& getOpponentMonsterZone() const; // zona mostri dell'avversario (read-only)
    std::vector<Card>& getGraveyard();
    std::vector<Card>& getOpponentGraveyard();
    std::vector<Card>& getGraveyardOf(int playerIdx);
    // Accesso a entrambe le mani (per effetti): 0 = current, 1 = opponent rispetto al turno corrente
    std::vector<Card>& getHandOf(int playerIdx);
    // Accesso al Deck (copia const) per ricerca; e helper per aggiungere carte in mano
    const Deck& getDeckOf(int playerIdx) const;
    Deck& getDeckOf(int playerIdx);
    void addToHand(int playerIdx, Card&& c);
    void discardExcess(size_t handLimit);
    // Estrae le carte oltre il limite (le rimuove dalla mano ma non le mette ancora nel graveyard)
    std::vector<Card> extractExcessCards(size_t handLimit);

    // Helper di sincronizzazione: quando effetti rimuovono carte dal Deck logico (non tramite DrawController),
    // riflette la rimozione anche nel Deck esterno usato dalla UI (se collegato).
    // Rimuove visivamente dal Deck esterno (UI) la prima carta con il nome indicato.
    // Se `sourceDeck` è fornito, la rimozione sul Deck esterno avviene solo se
    // `externalDeck` non punta alla stessa istanza, per evitare rimozioni duplicate.
    void mirrorExternalDeckRemoveByName(const std::string& name, const Deck* sourceDeck = nullptr);

    // Helper per End Phase: estrae le carte in eccesso dalla mano (usato dall'animazione di scarto esterna)
    std::vector<Card> handleEndPhase(size_t handLimit);

    // Battle API (v1)
    bool canDeclareAttack(size_t attackerIndex, std::optional<size_t> targetIndex) const;
    bool declareAttack(size_t attackerIndex, std::optional<size_t> targetIndex);
    // UI helper: true se il mostro nella zona indicata (giocatore corrente) ha già attaccato questo turno
    bool hasMonsterAlreadyAttacked(size_t zoneIndex) const;

    // Posizione/Set
    bool tryNormalSet(size_t handIndex); // posiziona in difesa coperta, richiede tributi come una Normal Summon
    // Slot-aware Set
    bool tryNormalSetToSlot(size_t handIndex, size_t slotIndex);
    bool setPosition(size_t zoneIndex, bool defense, bool faceDown = false, bool allowByEffect = false); // cambia posizione del mostro (rispetta fasi, salvo effetti)
    // Nuova API: cambia posizione secondo le regole
    // - Se coperto: scopri e metti in attacco
    // - Se in attacco: metti in difesa
    // - Se in difesa: metti in attacco
    bool togglePosition(size_t zoneIndex, bool allowByEffect = false);
    bool isDefenseAt(int playerIdx, size_t zoneIndex) const;
    bool isFaceDownAt(int playerIdx, size_t zoneIndex) const;
    // Mapping slot logico per il rendering stabile: ritorna lo slot [0..MONSTER_ZONE_SIZE-1] assegnato alla carta in monsterZones[playerIdx][zoneIndex]
    int getMonsterSlotIndexAt(int playerIdx, size_t zoneIndex) const;

    // Magie/Trappole
    static constexpr size_t ST_ZONE_SIZE = 3;
    // Set di Magie/Trappole dalla mano nella zona M/T (coperta). Per Field Spell usare setFieldSpell.
    bool setSpellOrTrap(size_t handIndex); // determina Spell/Trap in base ad Attribute/Type e setta coperta
    // Set di Magia Terreno (esclusiva).
    bool setFieldSpell(size_t handIndex);
    // Attiva una Magia dalla mano (Normale/Terreno/Rituale) se consentito; le Rapide dalla mano sono consentite.
    bool activateSpellFromHand(size_t handIndex);
    // Attiva una Trappola già settata (solo dal turno successivo al set del suo proprietario); analogo per Magie Rapide settate.
    bool activateSetSpellTrap(size_t zoneIndex);
    // Nuova API: come sopra ma specificando il proprietario assoluto (0 = P1 basso, 1 = P2 alto)
    // Consente anche l'attivazione nel turno avversario per Trappole e Magie Rapide settate (non nello stesso turno del set)
    bool activateSetSpellTrapFor(int playerIdx, size_t zoneIndex);
    // Verifica se una M/T settata e' attivabile senza alterare lo stato (gating identico all'attivazione)
    bool canActivateSetSpellTrapFor(int playerIdx, size_t zoneIndex);
    // Query stato M/T
    const std::vector<Card>& getSpellTrapZone() const { return spellTraps[turn.getCurrentPlayerIndex()]; }
    const std::vector<Card>& getOpponentSpellTrapZone() const { return spellTraps[1 - turn.getCurrentPlayerIndex()]; }
    bool isSpellTrapFaceDownAt(int playerIdx, size_t zoneIndex) const;
    bool isSpellTrapSetThisTurnAt(int playerIdx, size_t zoneIndex) const;
    // Field Spell getter (copia opzionale della carta presente nello slot Campo del giocatore indicato)
    std::optional<Card> getFieldSpellOf(int playerIdx) const { 
        if(playerIdx < 0 || playerIdx > 1) return std::nullopt; 
        return fieldSpell[playerIdx]; 
    }

    // Evocazione Speciale: inserisce una carta direttamente nella zona mostri del giocatore indicato
    // Restituisce false se la zona è piena o gli indici non sono validi
    bool specialSummonToMonsterZone(int playerIdx, Card&& c, bool defense, bool faceDown);
    // Nuova API: Evocazione Speciale con scelta posizione (Attacco o Difesa, mai coperta)
    // Flusso: l'effetto chiama requestSpecialSummonWithChoice(owner, card). Questo emette
    // un evento SpecialSummonChoiceRequested. L'UI risponde chiamando resolvePendingSpecialSummon(choiceDefense).
    bool requestSpecialSummonWithChoice(int playerIdx, Card&& c);
    bool hasPendingSpecialSummon() const { return pendingSS.has_value(); }
    // Returns the owner index of the pending special summon if present
    std::optional<int> getPendingSpecialSummonOwner() const;
    // choiceDefense: true = Difesa scoperta, false = Attacco scoperto
    bool resolvePendingSpecialSummon(bool choiceDefense);

    // Selezione carta dal Deck da inviare al Cimitero (per effetti come Reliquario del Drago)
    // Flusso: l'effetto chiama requestSendFromDeck(ownerIdx, predicateDescription), la UI mostra una lista filtrata
    // L'UI risponde chiamando resolvePendingSendFromDeck(indexInFiltered) oppure cancelPendingSendFromDeck.
    bool requestSendFromDeck(int playerIdx);
    bool hasPendingSendFromDeck() const { return pendingSend.has_value(); }
    // Returns the owner index for a pending send-from-deck request, if any
    std::optional<int> getPendingSendOwner() const;
    // indexInFiltered: indice nell'elenco filtrato di carte valide fornito dalla UI; ritorna false se out of range o fallisce
    bool resolvePendingSendFromDeck(size_t indexInFiltered);
    void cancelPendingSendFromDeck();

    // Scelta carta dalla mano da scartare (per costi effetti)
    bool requestHandDiscard(int playerIdx);
    bool hasPendingHandDiscard() const { return pendingDiscard.has_value(); }
    std::optional<int> getPendingHandDiscardOwner() const;
    bool resolvePendingHandDiscard(size_t handIndex);
    void cancelPendingHandDiscard();

    // Scelta carta dal Deck da aggiungere alla mano (search)
    bool requestAddFromDeck(int playerIdx);
    bool hasPendingAddFromDeck() const { return pendingAdd.has_value(); }
    std::optional<int> getPendingAddOwner() const;
    bool resolvePendingAddFromDeck(size_t indexInFiltered);
    void cancelPendingAddFromDeck();
    
    // Melodia: consenti fino a due aggiunte consecutive dopo lo scarto
    void setMelodiaAddsRemaining(int n){ melodiaAddsRemaining = n; }
    int getMelodiaAddsRemaining() const { return melodiaAddsRemaining; }
    void clearMelodiaAdds(){ melodiaAddsRemaining = 0; }

    // Sequenza generica: dopo uno scarto dalla mano, richiedi una scelta di aggiunta dal Deck per il giocatore indicato
    void setNextAddFromDeckAfterDiscard(int ownerIdx){ nextAddAfterDiscardOwner = ownerIdx; }

    // Helper per il debug
    void debugAddMonsterToOpponent(const Card& c);

    // Scelta carta dal Cimitero (per effetti come Richiamo del Posseduto)
    // Flusso: request -> UI mostra il Cimitero del proprietario e torna un indice; resolve rimuove la carta e avvia una SS con scelta posizione
    bool requestSelectFromGraveyard(int playerIdx);
    bool hasPendingGraveyardChoice() const { return pendingGY.has_value(); }
    std::optional<int> getPendingGraveyardOwner() const;
    bool resolvePendingSelectFromGraveyard(size_t indexInGY);
    void cancelPendingSelectFromGraveyard();

    // Sistema effetti (minimale)
    // Registra un'implementazione per un nome carta; quando tale carta è sul campo, verrà invocato onEvent
    void registerEffectForCardName(const std::string& cardName, std::unique_ptr<ICardEffect> effect);
    // Registra effetto di attivazione per Magie/Trappole o carte con attivazione esplicita
    void registerActivationForCardName(const std::string& cardName, std::unique_ptr<IActivationEffect> effect);

    // Ultima carta mandata al Cimitero (nome e proprietario logico 0/1 rispetto all'indice assoluto giocatore)
    struct LastSentToGrave { std::string name; int ownerIdx; };
    std::optional<LastSentToGrave> getLastSentToGrave() const { return lastSentToGrave; }
    void clearLastSentToGrave() { lastSentToGrave.reset(); }

    // --- Selezione di un proprio mostro sul terreno (per effetti come Saggio) ---
    enum class OwnMonsterFollowup { None, SageSacrifice };
    bool requestSelectOwnMonsterOnField(int ownerIdx, OwnMonsterFollowup followup);
    bool hasPendingOwnMonsterChoice() const { return pendingOwnMonster.has_value(); }
    std::optional<int> getPendingOwnMonsterOwner() const;
    bool resolvePendingSelectOwnMonster(size_t indexInOwnerZone);
    void cancelPendingSelectOwnMonster();

    // --- Aggiunta: Drago di Livello 1 dal Deck (per Saggio on-summon) ---
    bool requestAddLevel1DragonFromDeck(int playerIdx);
    bool hasPendingAddLevel1Dragon() const { return pendingAddLvl1.has_value(); }
    std::optional<int> getPendingAddLevel1DragonOwner() const;
    bool resolvePendingAddLevel1Dragon(size_t indexInFiltered);
    void cancelPendingAddLevel1Dragon();

    // --- Attivazione effetto Mostro dalla mano (Saggio) ---
    bool canActivateMonsterFromHand(size_t handIndex);
    bool activateMonsterFromHand(size_t handIndex);

    // --- Saggio: scelta BEWD/Spirit da mano/Deck/Cimitero e SS con scelta posizione ---
    // Avvia la richiesta di scelta candidati tra mano/deck/cimitero dell'owner
    bool requestBlueEyesOrSpiritChoice(int ownerIdx);
    bool hasPendingBlueEyesSSChoice() const { return pendingBlueEyesChoice.has_value(); }
    // Ritorna l'owner della pending scelta BE/Spirit
    std::optional<int> getPendingBlueEyesSSOwner() const { return pendingBlueEyesChoice ? std::optional<int>(pendingBlueEyesChoice->ownerIdx) : std::nullopt; }
    // Risolve prendendo l'indice nella lista mostrata (handFirst, poi deck, poi gy); esegue SS con scelta posizione via pendingSS
    bool resolvePendingBlueEyesOrSpiritChoice(size_t indexInFlattened);
    void cancelPendingBlueEyesOrSpiritChoice();

    // Avvia una chain interattiva a partire da un'activation request; la UI dovrebbe aprire la response window
    void startChainWithActivation(const std::string& cardName, int ownerIdx);
    // Il giocatore playerIdx risponde con una attivazione (nome carta). Ritorna true se accettata e aggiunta alla catena
    bool playerRespondWithActivation(int playerIdx, const std::string& cardName);
    // Il giocatore playerIdx passa (non risponde). Se entrambi i giocatori passano consecutivamente, la chain viene risolta LIFO
    void playerPassOnChain(int playerIdx);
    // Restituisce true se c'è una chain attiva in corso
    bool isChainActive() const { return chainActive; }
    // Tracciamento minimale per trigger "mandata al Cimitero"
    std::optional<LastSentToGrave> lastSentToGrave;

    // Stato pending per Evocazioni Speciali con scelta
    struct PendingSS { int ownerIdx; Card card; };
    std::optional<PendingSS> pendingSS;

    // Stato pending per selezione invio dal Deck al Cimitero
    struct PendingSendDeck { int ownerIdx; };
    std::optional<PendingSendDeck> pendingSend;

    // Stato pending per scelta scarto dalla mano
    struct PendingDiscardHand { int ownerIdx; };
    std::optional<PendingDiscardHand> pendingDiscard;
    // Stato pending per scelta aggiunta alla mano dal Deck
    struct PendingAddDeck { int ownerIdx; };
    std::optional<PendingAddDeck> pendingAdd;
    // Stato pending per scelta bersaglio dal Cimitero
    struct PendingGY { int ownerIdx; };
    std::optional<PendingGY> pendingGY;
    // Stato pending per scelta mostro proprio sul terreno
    struct PendingOwnMonster { int ownerIdx; OwnMonsterFollowup followup; };
    std::optional<PendingOwnMonster> pendingOwnMonster;
    // Stato pending per scelta BEWD/Spirit post Saggio: conserva owner e snapshot dei candidati
    struct PendingBlueEyesChoice { int ownerIdx; std::vector<Card> fromHand; std::vector<Card> fromDeck; std::vector<Card> fromGY; };
    std::optional<PendingBlueEyesChoice> pendingBlueEyesChoice;
    // Stato pending per aggiunta dal Deck di un Drago di Livello 1
    struct PendingAddLvl1 { int ownerIdx; };
    std::optional<PendingAddLvl1> pendingAddLvl1;
    // Azione da eseguire automaticamente dopo resolvePendingHandDiscard
    std::optional<int> nextAddAfterDiscardOwner;
    // Contatore per La Melodia (aggiunte rimanenti dopo lo scarto): opzionale, 0 = disattivo
    int melodiaAddsRemaining = 0;

    // Stato pending per Normal Summon con tributi: slot scelto
    std::optional<size_t> pendingSummonSlotIndex;

    // Overload interno per iniziare una Normal Summon con slot scelto
    bool beginNormalSummonWithTributes(size_t handIndex, bool asSet, std::optional<size_t> slotIndexOpt);

private:
    void checkVictoryByLP();
    void handleDeckOutIfAny();
    void processEnterEndPhase();
    void processEnterStandby();
    std::array<Player,2> players;
    TurnManager turn{0, GamePhase::Draw};
    int startingPlayerIndex = 0; // 0 = Player 1
    bool started = false;
    bool firstTurn = true; // per saltare pescata iniziale giocatore che inizia
    bool normalSummonUsed = false;
    static constexpr size_t MONSTER_ZONE_SIZE = 3;
    std::array<std::vector<Card>,2> monsterZones; // una per player
    // Slot mapping per ciascun mostro nella zona: monsterSlotIndex[p][i] = slot grafico assegnato a monsterZones[p][i]
    std::array<std::vector<int>,2> monsterSlotIndex;
    // Placeholder per Banished; Extra non gestito qui (UI lo tiene separato nel Deck)
    std::array<std::vector<Card>,2> banished;
    std::array<std::vector<Card>,2> graveyard; 
    
    EventDispatcher dispatcher;
    DrawController* drawCtrl = nullptr; // non-owning
    size_t handLimit = 7; // default
    std::function<void(std::vector<Card>&&)> discardCallback; // avvio animazione scarti
    bool pendingAutoTurnEnd = false; // End Phase senza scarti -> chiudere turno dopo un frame
    // Stato per Normal Summon con tributi
    std::optional<size_t> pendingSummonHandIndex; // indice carta in mano da evocare dopo scelta tributi
    int pendingTributesNeeded = 0;
    bool pendingSummonIsSet = false; // true se, dopo i tributi, si deve eseguire un Set

    // Stato battaglia: flag "ha attaccato" per ciascun mostro in zona, per giocatore
    std::array<std::vector<bool>,2> monsterHasAttacked;
    // Stato posizione per ciascun mostro (per giocatore): difesa/coperto
    std::array<std::vector<bool>,2> monsterIsDefense;
    std::array<std::vector<bool>,2> monsterIsFaceDown;
    // Stato Spell/Trap per ciascun giocatore
    std::array<std::vector<Card>,2> spellTraps; // zona M/T
    std::array<std::vector<bool>,2> stIsFaceDown; // true se coperta
    std::array<std::vector<bool>,2> stSetThisTurn; // true se settata in questo turno del proprietario
    std::array<std::optional<Card>,2> fieldSpell; // 1 slot per Field Spell per giocatore
    // Stato evocazione e cambio posizione per turno
    // true se il mostro è stato evocato (Normal Summon/Set) in QUESTO turno del suo controllore
    std::array<std::vector<bool>,2> monsterSummonedThisTurn;
    // true se il mostro ha già cambiato posizione in QUESTO turno del suo controllore
    std::array<std::vector<bool>,2> monsterPositionChangedThisTurn;

    // Stato vittoria
    bool gameOver = false;
    std::optional<int> winnerIndex; // indice del vincitore: 0/1

    // Puntatori ai Deck usati dal layer UI per ciascun giocatore (non-owning)
    std::array<Deck*,2> externalDecks{nullptr, nullptr};

    // Helper per la battaglia
    void destroyMonster(int playerIdx, size_t zoneIndex);
    void dealDamageTo(int playerIdx, int amount);
    // Helper slot
    bool isMonsterSlotOccupied(int playerIdx, size_t slotIndex) const;
    std::optional<size_t> firstFreeMonsterSlot(int playerIdx) const;

    // Helper per la propagazione degli effetti
    void dispatchEffects(GameEventType type);
    // Sistema effetti dedicato
    EffectSystem effects;
    // Stato per gestione interattiva della catena (response window)
    bool chainActive = false; // true se siamo in una response window
    int lastResponder = -1; // ultimo giocatore che ha risposto (0/1), -1 se nessuno

    // Flag di stato temporaneo (fino a fine turno) per annullare gli effetti dei mostri avversari
    // negateOppMonsterEffUntilEndOfTurn[p] == true significa: gli effetti dei mostri del suo avversario sono annullati fino a fine turno
    std::array<bool,2> negateOppMonsterEffUntilEndOfTurn{false, false};

public:
    // Applica l'effetto di "Rivali Predestinati" per il giocatore ownerIdx
    void applyDestinedRivalsFor(int ownerIdx) { if(ownerIdx>=0 && ownerIdx<=1) negateOppMonsterEffUntilEndOfTurn[ownerIdx] = true; }
    // Query: ritorna true se, per il giocatore playerIdx, gli effetti dei mostri avversari sono annullati in questo turno
    bool areOpponentMonsterEffectsNegatedFor(int playerIdx) const { return (playerIdx>=0 && playerIdx<=1) ? negateOppMonsterEffUntilEndOfTurn[playerIdx] : false; }

public:
    // Contesto dell'ultimo attacco dichiarato (per effetti che reagiscono a "viene scelto come bersaglio di un attacco")
    struct AttackContext { int attackerOwner; size_t attackerIndex; int defenderOwner; std::optional<size_t> targetIndex; };
    // Ultimo attacco dichiarato (per payload verso il sistema effetti)
    std::optional<AttackContext> lastDeclaredAttack;
    // Ritorna il contesto dell'ultimo attacco dichiarato (se presente)
    std::optional<AttackContext> getLastDeclaredAttack() const { return lastDeclaredAttack; }

public:
    
};
