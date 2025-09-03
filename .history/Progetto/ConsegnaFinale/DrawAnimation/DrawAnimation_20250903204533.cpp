#include "DrawAnimation.h"
#define PAUSE_DURATION 3.f

DrawAnimation::DrawAnimation(Card drawnCard, DrawAnimationPhases phase, sf::Vector2f startPos, sf::Vector2f pausePos, TextureManager& textureManager, bool skipCenterReveal)
    : card(drawnCard), phase(phase), startPos(startPos), pausePos(pausePos), constructedSkipCenterReveal(skipCenterReveal) {
    // Il riferimento a TextureManager può essere usato se serve per l'inizializzazione
}

bool DrawAnimation::moveTowards(sf::Vector2f& current, const sf::Vector2f& target, float speed, float deltaTime, Card& card) {
    sf::Vector2f direction = target - current;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance < 2.0f) {
        current = target;
        card.setPosition(current);
        return true;
    }

    direction /= distance;
    current += direction * speed * deltaTime;
    card.setPosition(current); 
    return false;
}


DrawAnimationPhases DrawAnimation::update(float moveSpeed, float deltaTime, sf::Texture& texture, std::vector<Card>& cards, sf::Vector2u& windowSize, sf::Vector2f& cardSize, float spacing, float y, int HAND_MAXSIZE, bool skipPause, bool skipCenterReveal, TextureManager& textureManager) {
    switch(phase){
        case DrawAnimationPhases::MovingOut: {
            // Se dobbiamo saltare la rivelazione (opponent draw), manda direttamente verso la mano con texture coperta
                if (constructedSkipCenterReveal || skipCenterReveal) {
                card.setSize(cardSize);
                card.setTexture(texture); // Back
                setHandPos(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                phase = DrawAnimationPhases::MovingHand;
                return phase;
            }
            sf::Vector2f& current = card.getPositionRef();
            if(current.y < static_cast<float>(windowSize.y)) { 
                current.y = current.y + moveSpeed * deltaTime;
                card.setPosition(current);
            } else { 
                current.y = static_cast<float>(windowSize.y);
                card.setPosition(current);
                // Imposta la fase successiva
                phase = DrawAnimationPhases::MovingToPause;
            }
            return phase;
        }

        case DrawAnimationPhases::MovingToPause: {
            card.setSize(cardSize);
            if (constructedSkipCenterReveal || skipCenterReveal) {
                // Salta completamente la pausa: mantieni texture coperta e vai direttamente verso la mano
                card.setTexture(texture); // texture coperta passata dall'esterno
                setHandPos(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                phase = DrawAnimationPhases::MovingHand;
                return phase;
            }
            // Usa la texture originale della carta (mostra al centro solo per P1)
            sf::Texture& originalTex = textureManager.getTexture(card.originalTexturePath);
            card.setTexture(originalTex);
            sf::Vector2u texSize = originalTex.getSize();
            card.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texSize.x, texSize.y)));

            sf::Vector2f& current = card.getPositionRef();
            if(moveTowards(current, pausePos, moveSpeed * 2.f, deltaTime, card)){
                phase = DrawAnimationPhases::ShowCard;
            }
            return phase;
        }

        case DrawAnimationPhases::ShowCard: {
            pauseTime += deltaTime;
            // skipPause è legato al click del giocatore: consente di saltare l'attesa
            if (pauseTime >= PAUSE_DURATION || skipPause) {
                phase = DrawAnimationPhases::MovingHand;
                setHandPos(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
                // resetta per la prossima animazione
                pauseDurationSet = false;
                pauseTime = 0.f;
            }
            return phase;
        }
        case DrawAnimationPhases::MovingHand: {
            if(moveTowards(card.getPositionRef(), handPos, moveSpeed * 3.f, deltaTime, card)){
                phase = DrawAnimationPhases::Done; 
                finished = true; 
            }
            return phase;
        }
        case DrawAnimationPhases::Done: {
            return phase;
        }
    }
}

void DrawAnimation::setHandPos(std::vector<Card>& cards, sf::Vector2u& windowSize, sf::Vector2f& cardSize, float spacing, float y, int HAND_MAXSIZE) {
    // Calcolo la posizione finale di card nella mano per far ciò inserisco momentaneamente la carta nella mano
    cards.push_back(card);
    updateHandPositions(cards, windowSize, cardSize, spacing, y, HAND_MAXSIZE);
    handPos = cards.back().getPosition();
    cards.pop_back(); // Rimuovo la carta dalla mano per non disegnarla due volte
}

void DrawAnimation::draw(sf::RenderWindow& window) {
    card.draw(window);
}

bool DrawAnimation::isFinished() const {
    return finished;
}

Card& DrawAnimation::getCard() const {
    return const_cast<Card&>(card); // Ritorno una reference non costante alla carta, per poterla modificare se necessario
}
