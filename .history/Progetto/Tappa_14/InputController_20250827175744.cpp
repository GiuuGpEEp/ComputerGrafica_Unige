#include "InputController.h"

using namespace Input;

static inline bool isRight(const sf::Event& e){
    if(const auto* mb = e.getIf<sf::Event::MouseButtonPressed>())
        return mb->button == sf::Mouse::Button::Right;
    return false;
}

static inline bool isLeft(const sf::Event& e){
    if(const auto* mb = e.getIf<sf::Event::MouseButtonPressed>())
        return mb->button == sf::Mouse::Button::Left;
    return false;
}

bool Controller::handleEvent(const sf::Event& e){
    // Chiusura finestra
    if (e.is<sf::Event::Closed>()) {
        ctx_.window.close();
        return true;
    }

    // Stato Game Over: gestisci solo conferma ritorno
    if(ctx_.gameOverActive){
        if(const auto* kp = e.getIf<sf::Event::KeyPressed>()){
            if(kp->code == sf::Keyboard::Key::Space || kp->code == sf::Keyboard::Key::Enter || kp->code == sf::Keyboard::Key::Escape){
                ctx_.gamestate = GameState::HomeScreen;
                ctx_.gameOverActive = false;
                if(ctx_.resetMatch) ctx_.resetMatch();
            }
        }
        return true; // blocca altri eventi durante game over
    }

    // Popup conferma ritorno alla Home
    if(ctx_.returnPopupActive){
        if(const auto* kp = e.getIf<sf::Event::KeyPressed>()){
            if(kp->code == sf::Keyboard::Key::Y){
                ctx_.gamestate = GameState::HomeScreen;
                ctx_.returnPopupActive = false;
                if(ctx_.resetMatch) ctx_.resetMatch();
            } else if(kp->code == sf::Keyboard::Key::N || kp->code == sf::Keyboard::Key::Escape || kp->code == sf::Keyboard::Key::Space){
                ctx_.returnPopupActive = false;
            }
            return true;
        }

        if(isLeft(e)){
            // Delega hit-test a RenderUtils (già usato in main)
            switch(RenderUtils::confirmReturnHomeHitTest(ctx_.windowSize, sf::Mouse::getPosition(ctx_.window))){
                case RenderUtils::ConfirmHit::Yes:
                    ctx_.gamestate = GameState::HomeScreen;
                    ctx_.returnPopupActive = false;
                    if(ctx_.resetMatch) ctx_.resetMatch();
                    break;
                case RenderUtils::ConfirmHit::No:
                    ctx_.returnPopupActive = false;
                    break;
                default: break;
            }
            return true;
        }
        return true; // assorbi tutto mentre il popup è attivo
    }

    // Invio dalla StartScreen
    if(const auto* kp = e.getIf<sf::Event::KeyPressed>()){
        if(kp->code == sf::Keyboard::Key::Enter && ctx_.gamestate == GameState::StartScreen){
            ctx_.gamestate = GameState::HomeScreen;
            return true;
        }
    }

    // Navigazione deck selection (ESC / Right click -> fade out)
    if(ctx_.gamestate == GameState::DeckSelection){
        if(const auto* kp2 = e.getIf<sf::Event::KeyPressed>()){
            if(kp2->code == sf::Keyboard::Key::Escape){
                if(!ctx_.deckSelectionScreen.isFading()) ctx_.deckSelectionScreen.startFadeOut(AppConfig::Timing::FadeOutFastSec);
                return true;
            }
        }
        if(isRight(e)){
            if(!ctx_.deckSelectionScreen.isFading()) ctx_.deckSelectionScreen.startFadeOut(AppConfig::Timing::FadeOutFastSec);
            return true;
        }
    }

    // Overlay DeckSend: click selezione cella
    if(ctx_.deckSendChoiceActive && isLeft(e)){
        if(auto hit = RenderUtils::deckSendHitIndex(ctx_.windowSize, ctx_.slotSize, ctx_.deckSendCandidates.size(), sf::Mouse::getPosition(ctx_.window))){
            ctx_.deckSendSelected = *hit;
        }
        return true; // consumato
    }

    // Battle: right click per scegliere bersaglio
    if(isRight(e) && ctx_.gamestate == GameState::Playing && !ctx_.inputBlocked()){
        Game* game = ctx_.getGame ? ctx_.getGame() : nullptr;
        if(game && game->getTurn().getPhase() == GamePhase::Battle && ctx_.selectedAttackerIndex.has_value()){
            sf::Vector2i mpos = sf::Mouse::getPosition(ctx_.window);
            for(size_t i=0;i<ctx_.oppFieldCards.size();++i){
                if(ctx_.oppFieldCards[i].isClicked(mpos)){
                    if(game->canDeclareAttack(ctx_.selectedAttackerIndex.value(), i)){
                        if(game->declareAttack(ctx_.selectedAttackerIndex.value(), i)){
                            ctx_.battleFeedbackMsg = "Attacco dichiarato";
                            ctx_.battleFeedbackClock.restart();
                        }
                    } else {
                        ctx_.battleFeedbackMsg = "Bersaglio non valido";
                        ctx_.battleFeedbackClock.restart();
                    }
                    break;
                }
            }
            return true;
        }
    }

    return false; // non gestito qui
}
