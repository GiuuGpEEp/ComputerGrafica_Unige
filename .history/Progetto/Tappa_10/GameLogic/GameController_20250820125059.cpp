#include "GameController.h"
#include <iostream>

GameController::GameController(Field& fieldRef,
                               Deck& deckP1,
                               Deck& deckP2,
                               std::array<std::vector<Card>,2>& handsRef,
                               std::array<std::vector<Card>,2>& fieldRefs,
                               std::vector<DrawAnimation>& drawAnims,
                               int& cardsToDrawRef)
: field(fieldRef),
  deck1(deckP1),
  deck2(deckP2),
  hands(handsRef),
  fields(fieldRefs),
  animations(drawAnims),
  cardsToDraw(cardsToDrawRef)
{
    players[0].id = 0;
    players[1].id = 1;
    players[0].deck = &deck1;
    players[1].deck = &deck2;
    players[0].hand = &hands[0];
    players[1].hand = &hands[1];
    players[0].fieldCards = &fields[0];
    players[1].fieldCards = &fields[1];
}

void GameController::onEnterPlaying() {
    activePlayer = 0;
    phase = Phase::Draw;
    drawDoneThisTurn = false;
    normalSummonUsed = false;
    std::cout << "[GC] Inizio partita - Turno P" << activePlayer+1 << "\n";
}

void GameController::handleEvent(const sf::Event& ev) {
    if(const auto* key = ev.getIf<sf::Event::KeyPressed>()){
        if(key->code == sf::Keyboard::Tab) nextPhase();
        else if(key->code == sf::Keyboard::T) endTurn();
    }
    if(const auto* mb = ev.getIf<sf::Event::MouseButtonPressed>()){
        if(mb->button == sf::Mouse::Left){
            if((phase==Phase::Main1 || phase==Phase::Main2) && activePlayer==0){
                // Solo P1 (mano visibile) per ora
                auto& h = hands[0];
                sf::Vector2i mp = sf::Mouse::getPosition();
                for(size_t i=0;i<h.size();++i){
                    if(h[i].isClicked(mp)){
                        tryNormalSummon(i);
                        break;
                    }
                }
            }
        }
    }
}

void GameController::update(float){
    if(phase==Phase::Draw) doDrawPhase();
}

void GameController::drawUI(sf::RenderWindow& win, sf::Font& font){
    sf::RectangleShape panel(sf::Vector2f(560.f, 110.f));
    panel.setPosition(20.f,20.f);
    panel.setFillColor(sf::Color(0,0,0,150));
    panel.setOutlineColor(sf::Color(255,200,90,160));
    panel.setOutlineThickness(2.f);
    win.draw(panel);

    auto phaseToStr=[](Phase p){
        switch(p){
            case Phase::Draw: return "DRAW";
            case Phase::Main1:return "MAIN1";
            case Phase::Battle:return "BATTLE";
            case Phase::Main2:return "MAIN2";
            case Phase::End:  return "END";
        }
        return "?";
    };
    std::string line1 = "Turno P" + std::to_string(activePlayer+1) + "  Fase: " + phaseToStr(phase);
    std::string line2 = "LP P1: " + std::to_string(players[0].life) + "   LP P2: " + std::to_string(players[1].life);
    std::string line3 = "(TAB=Fase  T=Fine Turno)";

    sf::Text t1(font, line1, 24); t1.setPosition(32.f,30.f);
    sf::Text t2(font, line2, 22); t2.setPosition(32.f,60.f);
    sf::Text t3(font, line3, 18); t3.setPosition(32.f,88.f);
    for(auto* t : {&t1,&t2,&t3}){
        t->setFillColor(sf::Color::White);
        t->setOutlineColor(sf::Color::Black);
        t->setOutlineThickness(2.f);
        win.draw(*t);
    }
}

void GameController::startTurn(){
    phase = Phase::Draw;
    drawDoneThisTurn = false;
    normalSummonUsed = false;
    std::cout << "[GC] Nuovo turno P" << activePlayer+1 << "\n";
}

void GameController::nextPhase(){
    switch(phase){
        case Phase::Draw: phase=Phase::Main1; break;
        case Phase::Main1: phase=Phase::Battle; break;
        case Phase::Battle: phase=Phase::Main2; break;
        case Phase::Main2: phase=Phase::End; break;
        case Phase::End: endTurn(); return;
    }
    std::cout << "[GC] -> fase " << (int)phase << "\n";
}

void GameController::endTurn(){
    activePlayer = (activePlayer+1)%2;
    startTurn();
}

void GameController::doDrawPhase(){
    if(drawDoneThisTurn) return;
    PlayerState& ap = players[activePlayer];
    if(!ap.deck || ap.deck->isEmpty()){
        std::cout << "[GC] Deck P" << activePlayer+1 << " vuoto.\n";
        drawDoneThisTurn = true;
        return;
    }

    if(activePlayer==0){
        // usa animazione già esistente: incrementa coda
        ++cardsToDraw;
    } else {
        // pescata istantanea nascosta (non animata) – (in futuro: animazione separata)
        Card c = ap.deck->drawCard();
        ap.hand->push_back(c);
    }
    drawDoneThisTurn = true;
    std::cout << "[GC] P" << activePlayer+1 << " pesca.\n";
}

bool GameController::tryNormalSummon(size_t handIndex){
    if(activePlayer!=0) return false; // per ora solo P1
    if(normalSummonUsed) { std::cout << "[GC] Normal già usata.\n"; return false; }
    auto& h = *players[0].hand;
    if(handIndex>=h.size()) return false;

    // Trova primo slot mostri libero P1 (placeholder: supponi field.getMonsterSlotsForPlayer esista)
    auto slots = field.getMonsterSlotsForPlayer(P1); // Se non esiste, implementare nel Field.
    void* freeSlotPtr = nullptr;
    Slot* freeSlot = nullptr;
    for(auto* s : slots){
        if(!s->isOccupied()){ freeSlot = s; break; }
    }
    if(!freeSlot){
        std::cout << "[GC] Nessuno slot libero.\n";
        return false;
    }
    Card c = h[handIndex];
    h.erase(h.begin()+handIndex);
    sf::Vector2f slotSize = freeSlot->getSize();
    float scale = 0.85f;
    sf::Vector2f newSize(slotSize.x*scale, slotSize.y*scale);
    c.setSize(newSize);
    sf::Vector2f pos = freeSlot->getPosition();
    c.setPosition({pos.x + (slotSize.x-newSize.x)/2.f,
                   pos.y + (slotSize.y-newSize.y)/2.f});
    players[0].fieldCards->push_back(c);
    freeSlot->setOccupied(true);
    normalSummonUsed = true;
    std::cout << "[GC] Normal Summon P1.\n";
    return true;
}