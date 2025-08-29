#include "InputController.h"

using namespace Input;

static inline bool isRight(const sf::Event& e){
    if(const auto* mb = e.getIf<sf::Event::MouseButtonPressed>())
        return mb->button == sf::Mouse::Right;
    return false;
}

static inline bool isLeft(const sf::Event& e){
    if(const auto* mb = e.getIf<sf::Event::MouseButtonPressed>())
        return mb->button == sf::Mouse::Left;
    return false;
}

bool Controller::handleEvent(const sf::Event& e){
    // Chiusura finestra
    if (e.is<sf::Event::Closed>()) {
        state.window.close();
        return true;
    }

    // Stato Game Over: gestisci solo conferma ritorno
    if(state.gameOverActive){
        if(const auto* kp = e.getIf<sf::Event::KeyPressed>()){
            if(kp->code == sf::Keyboard::Space || kp->code == sf::Keyboard::Enter || kp->code == sf::Keyboard::Escape){
                state.gamestate = GameState::HomeScreen;
                state.gameOverActive = false;
                if(state.resetMatch) state.resetMatch();
            }
        }
        return true; // blocca altri eventi durante game over
    }

    // Popup conferma ritorno alla Home
    if(state.returnPopupActive){
        if(const auto* kp = e.getIf<sf::Event::KeyPressed>()){
            if(kp->code == sf::Keyboard::Y){
                state.gamestate = GameState::HomeScreen;
                state.returnPopupActive = false;
                if(state.resetMatch) state.resetMatch();
            } else if(kp->code == sf::Keyboard::N || kp->code == sf::Keyboard::Escape || kp->code == sf::Keyboard::Space){
                state.returnPopupActive = false;
            }
            return true;
        }

        if(isLeft(e)){
            // Delega hit-test a RenderUtils (già usato in main)
            using RU = RenderUtils;
            switch(RU::confirmReturnHomeHitTest(state.windowSize, sf::Mouse::getPosition(state.window))){
                case RU::ConfirmHit::Yes:
                    state.gamestate = GameState::HomeScreen;
                    state.returnPopupActive = false;
                    if(state.resetMatch) state.resetMatch();
                    break;
                case RU::ConfirmHit::No:
                    state.returnPopupActive = false;
                    break;
                default: break;
            }
            return true;
        }
        return true; // assorbi tutto mentre il popup è attivo
    }

    // Invio dalla StartScreen
    if(const auto* kp = e.getIf<sf::Event::KeyPressed>()){
        if(kp->code == sf::Keyboard::Enter && state.gamestate == GameState::StartScreen){
            state.gamestate = GameState::HomeScreen;
            return true;
        }
    }

    // Navigazione deck selection (ESC / Right click -> fade out)
    if(state.gamestate == GameState::DeckSelection){
        if(const auto* kp2 = e.getIf<sf::Event::KeyPressed>()){
            if(kp2->code == sf::Keyboard::Escape){
                if(!state.deckSelectionScreen.isFading()) state.deckSelectionScreen.startFadeOut(AppConfig::Timing::FadeOutFastSec);
                return true;
            }
        }
        if(isRight(e)){
            if(!state.deckSelectionScreen.isFading()) state.deckSelectionScreen.startFadeOut(AppConfig::Timing::FadeOutFastSec);
            return true;
        }
    }

    // Overlay DeckSend: click selezione cella
    if(state.deckSendChoiceActive && isLeft(e)){
        if(auto hit = RenderUtils::deckSendHitIndex(state.windowSize, state.slotSize, state.deckSendCandidates.size(), sf::Mouse::getPosition(state.window))){
            state.deckSendSelected = *hit;
        }
        return true; // consumato
    }

    // Battle: right click per scegliere bersaglio
    if(isRight(e) && state.gamestate == GameState::Playing && !state.inputBlocked()){
        Game* game = state.getGame ? state.getGame() : nullptr;
        if(game && game->getTurn().getPhase() == GamePhase::Battle && state.selectedAttackerIndex.has_value()){
            sf::Vector2i mpos = sf::Mouse::getPosition(state.window);
            for(size_t i=0;i<state.oppFieldCards.size();++i){
                if(state.oppFieldCards[i].isClicked(mpos)){
                    if(game->canDeclareAttack(state.selectedAttackerIndex.value(), i)){
                        if(game->declareAttack(state.selectedAttackerIndex.value(), i)){
                            state.battleFeedbackMsg = "Attacco dichiarato";
                            state.battleFeedbackClock.restart();
                        }
                    } else {
                        state.battleFeedbackMsg = "Bersaglio non valido";
                        state.battleFeedbackClock.restart();
                    }
                    break;
                }
            }
            return true;
        }
    }

    return false; // non gestito qui
}
