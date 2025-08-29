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
        state.window.close();
        return true;
    }

    // Stato Game Over: gestisci solo conferma ritorno
    if(state.gameOverActive){
        if(const auto* kp = e.getIf<sf::Event::KeyPressed>()){
            if(kp->code == sf::Keyboard::Key::Space || kp->code == sf::Keyboard::Key::Enter || kp->code == sf::Keyboard::Key::Escape){
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
            if(kp->code == sf::Keyboard::Key::Y){
                state.gamestate = GameState::HomeScreen;
                state.returnPopupActive = false;
                if(state.resetMatch) state.resetMatch();
            } else if(kp->code == sf::Keyboard::Key::N || kp->code == sf::Keyboard::Key::Escape || kp->code == sf::Keyboard::Key::Space){
                state.returnPopupActive = false;
            }
            return true;
        }

        if(isLeft(e)){
            // Delega hit-test a RenderUtils (già usato in main)
            switch(RenderUtils::confirmReturnHomeHitTest(state.windowSize, sf::Mouse::getPosition(state.window))){
                case RenderUtils::ConfirmHit::Yes:
                    state.gamestate = GameState::HomeScreen;
                    state.returnPopupActive = false;
                    if(state.resetMatch) state.resetMatch();
                    break;
                case RenderUtils::ConfirmHit::No:
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
        if(kp->code == sf::Keyboard::Key::Enter && state.gamestate == GameState::StartScreen){
            state.gamestate = GameState::HomeScreen;
            return true;
        }
    }

    // Navigazione Prompt di risposta da tastiera (no wrap, come in main originale)
    if(state.responsePromptActive){
        if(const auto* kp = e.getIf<sf::Event::KeyPressed>()){
            if(kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::Left){
                if(state.responseOptionsCount && state.responseOptionsCount()>0){ if(state.responseSelected>0) state.responseSelected--; }
                return true;
            }
            if(kp->code == sf::Keyboard::Key::Down || kp->code == sf::Keyboard::Key::Right){
                if(state.responseOptionsCount && state.responseOptionsCount()>0){ if(state.responseSelected + 1 < state.responseOptionsCount()) state.responseSelected++; }
                return true;
            }
            if(kp->code == sf::Keyboard::Key::Enter){
                if(state.confirmResponsePrompt && (!state.responseOptionsCount || state.responseOptionsCount()>0)){
                    state.confirmResponsePrompt(state.responseSelected);
                }
                return true;
            }
            if(kp->code == sf::Keyboard::Key::Escape){
                if(state.closeResponsePrompt) state.closeResponsePrompt();
                return true;
            }
        }
    }

    // Uscita dalla deck selection (ESC / Right click -> fade out)
    if(state.gamestate == GameState::DeckSelection){
        if(const auto* kp2 = e.getIf<sf::Event::KeyPressed>()){
            if(kp2->code == sf::Keyboard::Key::Escape){
                if(!state.deckSelectionScreen.isFading()) state.deckSelectionScreen.startFadeOut(AppConfig::Timing::FadeOutFastSec);
                return true;
            }
        }
        if(isRight(e)){
            if(!state.deckSelectionScreen.isFading()) state.deckSelectionScreen.startFadeOut(AppConfig::Timing::FadeOutFastSec);
            return true;
        }
    }

    // In gioco: SPACE apre il popup "Torna alla Home"
    if(const auto* kp3 = e.getIf<sf::Event::KeyPressed>()){
        if(kp3->code == sf::Keyboard::Key::Space && state.gamestate == GameState::Playing){
            state.returnPopupActive = true;
            return true;
        }
    }

    // ESC: gestione globale di chiusura/cancel
    if(const auto* kpEsc = e.getIf<sf::Event::KeyPressed>()){
        if(kpEsc->code == sf::Keyboard::Key::Escape){
            // Chiudi solo il prompt di risposta (non fare continue qui di proposito)
            if(state.responsePromptActive){
                if(state.closeResponsePrompt) state.closeResponsePrompt();
                // non return, lascia che altre logiche di ESC facciano effetto
            }
            // ESC chiude overlay scelta DeckSend
            if(state.deckSendChoiceActive){
                if(Game* g = state.getGame ? state.getGame() : nullptr){ g->cancelPendingSendFromDeck(); }
                state.deckSendChoiceActive = false; state.deckSendCandidates.clear(); state.deckSendSelected = 0; state.deckSendOwner = -1;
                return true; // consumato: è un'azione modale
            }

            // Se non siamo in DeckSelection gestiamo la deselezione/chiusure
            if(state.gamestate != GameState::DeckSelection){
                // Deseleziona carta e resetta scroll
                state.selectedCardIndex.reset();
                state.scrollOffset = 0.f;
                // Chiudi overlay extra/gy (tramite callback dal main)
                if(state.extraOnEscape) state.extraOnEscape();
                if(state.graveOnEscape) state.graveOnEscape();
                // Reset battle selection
                state.selectedAttackerIndex.reset();
                state.attackSelectionActive = false;
                // Se stiamo selezionando tributi: annulla selezione e summon pendente
                if(state.selectingTributes){
                    if(Game* g = state.getGame ? state.getGame() : nullptr){ g->cancelPendingNormalSummon(); }
                    state.selectingTributes = false; state.selectedTributes.clear(); state.tributesNeeded = 0;
                    state.battleFeedbackMsg.clear();
                    if(state.syncMonsterZoneToField) state.syncMonsterZoneToField();
                }
                if(state.attackSelectionActive){
                    state.attackSelectionActive = false;
                    state.selectedAttackerIndex.reset();
                    state.battleFeedbackMsg.clear();
                }
                if(state.chooseSummonOrSet || state.chooseActivateOrSetST){
                    state.chooseSummonOrSet = false;
                    state.chooseActivateOrSetST = false;
                    if(state.pendingHandIndexForPlay.has_value() && state.restoreCardAtHandIndex){
                        state.restoreCardAtHandIndex(state.pendingHandIndexForPlay.value());
                    }
                    state.pendingHandIndexForPlay.reset();
                    state.pendingIsSpellTrap = false;
                }
                return true;
            }
        }
    }

    // Overlay DeckSend: click selezione cella
    if(state.deckSendChoiceActive && isLeft(e)){
        if(auto hit = RenderUtils::deckSendHitIndex(state.windowSize, state.slotSize, state.deckSendCandidates.size(), sf::Mouse::getPosition(state.window))){
            state.deckSendSelected = *hit;
        }
        return true; // consumato
    }

    // Extra/Cimitero: gestione hold press/release (solo se input non bloccato)
    if(!state.inputBlocked() && (e.is<sf::Event::MouseButtonPressed>() || e.is<sf::Event::MouseButtonReleased>())){
        sf::Vector2f mousePosF = static_cast<sf::Vector2f>(sf::Mouse::getPosition(state.window));
        sf::FloatRect extraRect(state.extraDeckSlotPos, state.slotSize);
        sf::FloatRect gyRect(state.graveyardSlotPos, state.slotSize);
        if(const auto* mbp = e.getIf<sf::Event::MouseButtonPressed>()){
            if(mbp->button == sf::Mouse::Button::Left){
                // Avvia hold sugli slot se il mouse è dentro
                state.extraOverlay.startHold(mousePosF, extraRect, state.gamestate);
                bool gyHasCards = false;
                if(state.getGame){ 
                    if(Game* g = state.getGame()) gyHasCards = !g->getGraveyard().empty(); }
                state.graveyardOverlay.startHold(mousePosF, gyRect, state.gamestate, gyHasCards);
                // Click per aprire/chiudere lista carte overlay
                state.extraOverlay.handleMousePress(mousePosF, state.getExtraCards ? state.getExtraCards() : std::vector<Card>{});
                if(state.getGame){ state.graveyardOverlay.handleMousePress(mousePosF, state.getGraveyardCards ? state.getGraveyardCards() : std::vector<Card>{}); }
                return false; // lascia passare in caso servano altri handler del main
            }
        }
        if(e.is<sf::Event::MouseButtonReleased>()){
            state.extraOverlay.handleMouseRelease();
            state.graveyardOverlay.handleMouseRelease();
            return false;
        }
    }

    // Overlay DeckSend: conferma con tastiera e navigazione griglia (cols da AppConfig)
    if(state.deckSendChoiceActive){
        if(const auto* kp = e.getIf<sf::Event::KeyPressed>()){
            if(kp->code == sf::Keyboard::Key::Enter){
                if(state.confirmDeckSend) state.confirmDeckSend(state.deckSendSelected);
                return true;
            }
            if(kp->code == sf::Keyboard::Key::Left || kp->code == sf::Keyboard::Key::Up){
                if(!state.deckSendCandidates.empty()){
                    size_t cols = AppConfig::Ui::DeckSendCols;
                    if(kp->code == sf::Keyboard::Key::Left){
                        if(state.deckSendSelected>0) state.deckSendSelected--;
                    } else { // Up
                        if(state.deckSendSelected >= cols) state.deckSendSelected -= cols;
                    }
                }
                return true;
            }
            if(kp->code == sf::Keyboard::Key::Right || kp->code == sf::Keyboard::Key::Down){
                if(!state.deckSendCandidates.empty()){
                    size_t cols = AppConfig::Ui::DeckSendCols;
                    if(kp->code == sf::Keyboard::Key::Right){
                        if(state.deckSendSelected + 1 < state.deckSendCandidates.size()) state.deckSendSelected++;
                    } else { // Down
                        if(state.deckSendSelected + cols < state.deckSendCandidates.size()) state.deckSendSelected += cols;
                    }
                }
                return true;
            }
        }
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
