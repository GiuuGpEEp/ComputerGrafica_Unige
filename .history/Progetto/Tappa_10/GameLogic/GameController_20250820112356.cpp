#include "GameController.h"
#include <iostream>
#include <cmath>

GameController::GameController(Field& fieldRef,
                               std::vector<Card>& handRef,
                               std::vector<Card>& fieldCardsRef,
                               Deck& sharedDeck,
                               std::vector<DrawAnimation>& drawAnims,
                               int& cardsToDrawRef)
: field(fieldRef),
  hand(handRef),
  fieldCards(fieldCardsRef),
  deck(sharedDeck),
  animations(drawAnims),
  cardsToDraw(cardsToDrawRef)
{
    players[0].id = 0;
    players[1].id = 1;
    players[0].deck = &deck;
    players[0].hand = &hand;
    players[0].fieldCards = &fieldCards;
    // (Se in futuro aggiungi secondo deck/mani, popola players[1])
}

void GameController::onEnterPlaying() {
    activePlayer = 0;
    phase = Phase::Draw;
    drawDoneThisTurn = false;
    normalSummonUsed = false;
    std::cout << "[GameController] Inizio partita -> Turno Giocatore " << activePlayer+1 << std::endl;
}

void GameController::handleEvent(const sf::Event& ev) {
    if(const auto* key = ev.getIf<sf::Event::KeyPressed>()) {
        switch(key->code) {
            case sf::Keyboard::Key::Tab:
                nextPhase();
                break;
            case sf::Keyboard::Key::T:
                endTurn();
                break;
            default: break;
        }
    }
    // Esempio summon clic: (sinistro + fase Main1/Main2)
    if(const auto* mb = ev.getIf<sf::Event::MouseButtonPressed>()) {
        if(mb->button == sf::Mouse::Left) {
            if(phase == Phase::Main1 || phase == Phase::Main2) {
                // Trova carta in mano cliccata
                for(size_t i=0;i<hand.size();++i){
                    if(hand[i].isClicked(sf::Mouse::getPosition())) {
                        sf::Vector2f mpos = sf::Vector2f(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);
                        tryNormalSummon(i, mpos);
                        break;
                    }
                }
            }
        }
    }
}

void GameController::update(float dt) {
    (void)dt;
    switch(phase) {
        case Phase::Draw:   doDrawPhase(); break;
        case Phase::Battle: doBattlePhaseAttack(); break;
        default: break;
    }
}

void GameController::drawUI(sf::RenderWindow& win, sf::Font& font) {
    sf::RectangleShape bar(sf::Vector2f(480.f, 70.f));
    bar.setPosition(sf::Vector2f(20.f, 20.f));
    bar.setFillColor(sf::Color(0,0,0,140));
    bar.setOutlineColor(sf::Color(255,200,90,180));
    bar.setOutlineThickness(2.f);
    win.draw(bar);

    auto phaseToStr = [](Phase p){
        switch(p){
            case Phase::Draw: return "DRAW";
            case Phase::Main1:return "MAIN1";
            case Phase::Battle:return "BATTLE";
            case Phase::Main2:return "MAIN2";
            case Phase::End:  return "END";
        }
        return "?";
    };
    std::string txt = "Turno P" + std::to_string(activePlayer+1) + "  Fase: " + phaseToStr(phase) +
                      "  (TAB=Fase, T=Fine Turno)";
    sf::Text t(font, txt, 24);
    t.setPosition(sf::Vector2f(32.f, 35.f));
    t.setFillColor(sf::Color::White);
    t.setOutlineColor(sf::Color::Black);
    t.setOutlineThickness(2.f);
    win.draw(t);
}

void GameController::startTurn() {
    phase = Phase::Draw;
    drawDoneThisTurn = false;
    normalSummonUsed = false;
    std::cout << "[GameController] Nuovo turno P" << activePlayer+1 << std::endl;
}

void GameController::nextPhase() {
    switch(phase){
        case Phase::Draw: phase = Phase::Main1; break;
        case Phase::Main1: phase = Phase::Battle; break;
        case Phase::Battle: phase = Phase::Main2; break;
        case Phase::Main2: phase = Phase::End; break;
        case Phase::End: endTurn(); return;
    }
    std::cout << "[GameController] Fase -> " << (int)phase << std::endl;
}

void GameController::endTurn() {
    phase = Phase::Draw;
    activePlayer = (activePlayer + 1) % 2;
    drawDoneThisTurn = false;
    normalSummonUsed = false;
    std::cout << "[GameController] Cambio turno -> P" << activePlayer+1 << std::endl;
}

void GameController::doDrawPhase() {
    if(drawDoneThisTurn) return;
    if(deck.isEmpty()) {
        std::cout << "[GameController] Deck vuoto: condizione di fine partita da gestire." << std::endl;
        drawDoneThisTurn = true;
        return;
    }
    // Riusa pipeline animazione già esistente: incrementa coda
    if(animations.empty()) {
        // Innesca una pescata (usa stesso meccanismo di main: cardsToDraw++)
        ++cardsToDraw;
        drawDoneThisTurn = true;
        std::cout << "[GameController] P" << activePlayer+1 << " pesca." << std::endl;
    }
}

bool GameController::tryNormalSummon(size_t handIndex, const sf::Vector2f& mousePos) {
    if(normalSummonUsed) {
        std::cout << "[GameController] Normal Summon già usata." << std::endl;
        return false;
    }
    
    Slot* freeSlot = nullptr;

    

    auto monsterSlots = field.getMonsterSlotsForPlayer(1); 
    for(auto* s : monsterSlots){
        if(!s->isOccupied()){
            freeSlot = s;
            break;
        }
    }
    if(!freeSlot){
        std::cout << "[GameController] Nessuno slot libero." << std::endl;
        return false;
    }
    // Sposta carta dal vettore mano al campo
    if(handIndex >= hand.size()) return false;
    Card c = hand[handIndex];
    hand.erase(hand.begin()+handIndex);

    float fieldCardScale = 0.85f;
    sf::Vector2f slotSize = freeSlot->getSize(); // supponi esista
    sf::Vector2f newSize(slotSize.x*fieldCardScale, slotSize.y*fieldCardScale);
    c.setSize(newSize);
    sf::Vector2f slotPos = freeSlot->getPosition();
    sf::Vector2f centered(slotPos.x + (slotSize.x-newSize.x)/2.f,
                          slotPos.y + (slotSize.y-newSize.y)/2.f);
    c.setPosition(centered);
    fieldCards.push_back(c);
    freeSlot->setOccupied(true);
    normalSummonUsed = true;
    std::cout << "[GameController] Normal Summon eseguita." << std::endl;
    return true;
}

void GameController::doBattlePhaseAttack() {
    // Placeholder: in futuro selezione attaccante / bersaglio
    // Per ora auto-salta se non ci sono mostri
    if(fieldCards.empty()) return;
    // ...
}