#include <SFML/Graphics.hpp>
#include "Field/Field.h"
#include "Deck/Deck.h"
#include "../resources/GameState.h"
#include "auxFunc.h"
#include <vector>
#include <iostream>
#define P1 1 
#define P2 2
#define DECK_SIZE 30 // Numero di carte nel deck
#define HAND_MAXSIZE 7 

enum class DrawAnimationPhases{
    MovingOut,
    ShowCard,
    MovingHand,
    Done
};

struct CardAnimation{
    Card card;
    DrawAnimationPhases phase = DrawAnimationPhases::MovingOut;
    sf::Vector2f startPos;
    sf::Vector2f outScreenPos;
    sf::Vector2f pausePos; //Posizione di pausa, per mostrare la carta prima di spostarla in mano
    sf::Vector2f handPos; //Posizione finale in mano, calcolata con updateHandPositions
    float pauseTime = 0.f;
    bool finished = false; 
};

bool moveTowards(sf::Vector2f& current, const sf::Vector2f& target, float speed, float deltaTime) { 
    //Per avere un movimento fluido bisogna chiamarla più volte, frame per frame

    //Calcolo la direzione, e la distanza euclidea effettiva
    sf::Vector2f direction = target - current;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    //Se la distanza è minore della velocità * deltaTime, significa non posso più muovermi verso il target senza superarlo, quindi sono arrivato al target
    if (distance < speed * deltaTime) {
        current = target;
        return true;
    }

    direction /= distance; // Normalizzo, in modo da avere un vettore di lunghezza 1 
    current += direction * speed * deltaTime;
    return false;
}

int main(){

    std::vector<CardAnimation> animations;
    GameState gamestate = GameState::Intro;

    // Definisci le dimensioni della finestra
    sf::Vector2u windowSize(1920, 1080);
    sf::RenderWindow window(sf::VideoMode(windowSize), "Progetto Tappa 06 - Animationz");

    // Carico le texture necessarie per il campo di gioco
    sf::Texture fieldTexture("../../Progetto/resources/backgroundTexture.jpg");
    sf::Texture monsterTexture("../../Progetto/resources/monsterText.png");
    sf::Texture spellTrapTexture("../../Progetto/resources/spellTrapTexture.png");
    sf::Texture deckTexture("../../Progetto/resources/deckTexture.png");
    sf::Texture graveyardTexture("../../Progetto/resources/graveTexture.png");
    sf::Texture extraDeckTexture("../../Progetto/resources/ExtraDeckTexture.png");
    sf::Texture fieldSpellTexture("../../Progetto/resources/fieldSpell.png");

    // Carico le texture necessarie per le carte
    sf::Texture textureFlipped(("../../Progetto/resources/Texture1.png"));
    sf::Texture textureNonFlipped(("../../Progetto/resources/CardNotSet.jpg"));

    // Carico il font per le etichette delle carte
    sf::Font detailFont;
    if (!detailFont.openFromFile("../../Progetto/resources/ITCKabelStdDemi.TTF")) {

        if (!detailFont.openFromFile("C:/Windows/Fonts/calibri.ttf")) {
            std::cerr << "ERRORE: Impossibile caricare nessun font!" << std::endl;
        }
    }

    // Crea il campo di gioco con dimensioni dinamiche
    Field field(
        fieldTexture, 
        monsterTexture, 
        spellTrapTexture, 
        deckTexture, 
        graveyardTexture, 
        extraDeckTexture, 
        fieldSpellTexture, 
        windowSize
    );        
    
    // Calcola le dimensioni per le carte del deck (leggermente più piccole dello slot)
    sf::Vector2f slotSize = calculateSlotSize(windowSize);
    float deckScaleFactor = 0.9f; 
    sf::Vector2f deckCardSize(slotSize.x * deckScaleFactor, slotSize.y * deckScaleFactor);

    // Posizione dello slot del deck
    sf::Vector2f deckSlotPos = field.getSlotPosition(Slot::Type::Deck, P1); 

    // Creo il deck
    Deck deck(deckSlotPos, deckCardSize, slotSize, textureFlipped, DECK_SIZE);

    // Creo le carte in mano, inizialmente zero
    std::vector<Card> cards;
    cards.reserve(10); 
    const int initialCard = 8;

    //Calcolo la dimensione della carta rendendola un po' più grande delle dimensioni degli slot e prendo i parametri di posizione e spaziatura
    float scaleFactor = 1.1f;
    sf::Vector2f cardSize(slotSize.x * scaleFactor, slotSize.y * scaleFactor);
    float spacing = 15.f;
    float totalHandWidth = initialCard * cardSize.x + (initialCard - 1) * spacing;
    float startX = (windowSize.x - totalHandWidth) / 2.f;
    
    //Ripristino la posizione originale delle carte
    float y = windowSize.y - cardSize.y - 15.f; 

    std::optional<size_t> selectedCardIndex; // Indice della carta selezionata
    float scrollOffset = 0.f; //Offset per lo scroll del testo dei dettagli della carta

    //Creo un clock per far passare qualche secondo prima di mostrare il campo di gioco
    sf::Clock clock;


    while(window.isOpen()){

        //1. Gestione degli eventi
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()) {
                std::cout << "Richiesta di chiusura..." << std::endl;
                window.close();
            }

            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                if(keyPressed->code == sf::Keyboard::Key::Enter && gamestate == GameState::Intro) {
                    std::cout << "Passaggio allo stato FieldVisible..." << std::endl;
                    gamestate = GameState::FieldVisible; 
                    clock.restart(); // 
                }
            }

            if(gamestate == GameState::FieldVisible && clock.getElapsedTime().asSeconds() > 1.f) {
                std::cout << "Passaggio allo stato Playing..." << std::endl;
                gamestate = GameState::Playing; // Passa allo stato di gioco dopo 1 secondo

                //Pesco le carte iniziali
                for(int i=0; i<initialCard; ++i){
                    if(!deck.isEmpty()){
                        Card drawnCard = deck.drawCard();
                        CardAnimation anim{
                            drawnCard, 
                            DrawAnimationPhases::MovingOut, 
                            deckSlotPos, 
                            (deckSlotPos + sf::Vector2f(0, 150)),
                            (sf::Vector2f(windowSize.x / 2.f - cardSize.x / 2.f, windowSize.y / 2.f - cardSize.y / 2.f)),
                        };        
                        
                        animations.push_back(anim);
                    }
                }

                 // Ricalcola le posizioni dopo aver aggiunto le carte
                updateHandPositions(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
            }

            if (const auto *mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Mouse::Button but = mouseButton->button;
                if(but == sf::Mouse::Button::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                    bool cardClicked = false;
                    for (size_t i = 0; i < cards.size(); ++i) {
                        if (cards[i].isClicked(mousePos)) {
                            // Se è una carta diversa da quella già selezionata, resetta lo scroll
                            if (selectedCardIndex.has_value() && selectedCardIndex.value() != i) {
                                scrollOffset = 0.f; // Reset dello scroll
                            }

                            selectedCardIndex = i; // Aggiorna l'indice della carta selezionata
                            cardClicked = true;
                            std::cout << "Hai cliccato: " << cards[i].getName() << std::endl;
                            break;
                        }
                    }

                     // Se ho cliccato fuori dalle carte, deseleziona
                    if (!cardClicked) {
                        selectedCardIndex.reset();
                        scrollOffset = 0.f; // Resettando lo scroll quando seleziono fuori (o un'altra carta) ritorno in cima alla view
                    }
                }    
            }

            // Gestione tasto ESC per deselezionare
            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    selectedCardIndex.reset();
                    scrollOffset = 0.f; // Resetta lo scroll anche con ESC
                    std::cout << "Carta deselezionata con ESC" << std::endl;
                }
            }
             
            // Scroll mouse
            if (const auto* mouseScroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (selectedCardIndex.has_value()) {
                    scrollOffset -= mouseScroll->delta * 20.f;
                    scrollOffset = std::max(0.f, scrollOffset); // niente scroll negativo
                }
            }
        }
        //2. Aggiornamento della logica del gioco

        static sf::Clock clock;
        float deltaTime = clock.restart().asSeconds();

        if(gamestate == GameState::Playing){
            float moveSpeed = 800.f; // Velocità di movimento delle carte in pixel al secondo
            
            for(auto& anim : animations){
                switch(anim.phase){
                    case DrawAnimationPhases::MovingOut:
                        // Se sono nella fase di MovingOut vuol dire che la carta deve passare dal deck a fuori la mano
                        
                        if(moveTowards(anim.card.getPositionRef(), anim.outScreenPos, moveSpeed, deltaTime)) {
                            anim.phase = DrawAnimationPhases::ShowCard; // Passa alla fase di mostrare la carta
                            anim.card.setTexture(textureNonFlipped);
                            
                            sf::Vector2u texSize = textureNonFlipped.getSize();
                            anim.card.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texSize.x, texSize.y)));
                            anim.card.setPosition(anim.pausePos);
                        }
                        break;

                    case DrawAnimationPhases::ShowCard:
                        // Se sono nella fase di ShowCard, aspetto un po' di tempo prima di passare alla fase successiva e spostarla nella mano
                        anim.pauseTime += deltaTime;
                        if(anim.pauseTime >= 1.f){ //mostro la carta per 1 secondo e passo alla fase successiva
                            anim.phase = DrawAnimationPhases::MovingHand;

                            //Calcolo la posizione finale di anim.card nella mano per far ciò inserisco momentaneamente la carta nella mano
                            cards.push_back(anim.card);
                            updateHandPositions(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                            anim.handPos = cards.back().getPosition();
                            cards.pop_back(); // Rimuovo la carta dalla mano per non disegnarla due volte

                            anim.card.setSize(cardSize); 
                            sf::Vector2u texSize = textureNonFlipped.getSize();
                            anim.card.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texSize.x, texSize.y)));
                        } 
                        break;
                    case DrawAnimationPhases::MovingHand:
                        // Se sono nella fase di MovingHand, sposto la carta nella mano
                        
                        if(moveTowards(anim.card.getPositionRef(), anim.handPos, moveSpeed, deltaTime)){
                            anim.phase = DrawAnimationPhases::Done; 
                            anim.finished = true; 
                        }
                    break;
                    
                    case DrawAnimationPhases::Done:
                    break;    
                }            
            }

            //Quando un'animazione è finita, la rimuovo dalla lista 
            auto it = animations.begin(); // Inizializzo l'iteratore per scorrere le animazioni (begin serve per allocare le risorse necessarie)
            while (it != animations.end()) {
                if (it->finished) {
                    cards.push_back(it->card); 
                    it = animations.erase(it); // Rimuove l'animazione e aggiorna l'iteratore
                    updateHandPositions(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE); 
                } else {
                    ++it; // Altrimenti, passa all'animazione successiva
                }    
            }   
        }

        //3. Blocco Rendering
        window.clear(sf::Color::Black);

        // Ottieni la posizione del mouse
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        
        // Disegna sempre il campo di gioco (che include lo sfondo)
        field.draw(window, mousePos, gamestate);
        
        // Se siamo nello stato Intro, disegna il testo sopra lo sfondo
        if(gamestate == GameState::Intro) drawStartScreen(window, detailFont, windowSize);
        
        else {
            // Negli altri stati, disegna anche carte e deck
            for (auto& card : cards) card.draw(window);

            // Disegna il deck 
            deck.draw(window, mousePos, detailFont, deckSlotPos, slotSize, gamestate);

            //Disegna le animazioni delle carte
            for(auto& anim : animations) anim.card.draw(window);
            
            // Se c'è una carta selezionata, mostra i dettagli - Non influisce sullo stato del gioco, ma solo sulla visualizzazione (rendering)
            if (selectedCardIndex.has_value()) {
                const Card& selectedCard = cards[selectedCardIndex.value()];
                sf::Vector2f panelPos{400.f, 150.f};
                sf::Vector2f panelSize{300.f, 200.f};
                showCardDetails(window, selectedCard, detailFont, panelPos, panelSize, scrollOffset);
            }
        }
        
        window.display();
    }
    return 0;
}