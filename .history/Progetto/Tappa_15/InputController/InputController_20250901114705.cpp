#include "InputController.h"

using namespace Input;

static inline bool isRight(const sf::Event& e){
    if(const auto* mb = e.getIf<sf::Event::MouseButtonPressed>())
        return mb->button == M_MOUSE_RIGHT;
    return false;
}

static inline bool isLeft(const sf::Event& e){
    if(const auto* mb = e.getIf<sf::Event::MouseButtonPressed>())
        return mb->button == M_MOUSE_LEFT;
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
            if(kp->code == M_KEY_SPACE || kp->code == M_KEY_ENTER || kp->code == M_KEY_ESCAPE){
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
            if(kp->code == M_KEY_Y){
                state.gamestate = GameState::HomeScreen;
                state.returnPopupActive = false;
                if(state.resetMatch) state.resetMatch();
            } else if(kp->code == M_KEY_N || kp->code == M_KEY_ESCAPE || kp->code == M_KEY_SPACE){
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
        if(kp->code == M_KEY_ENTER && state.gamestate == GameState::StartScreen){
            state.gamestate = GameState::HomeScreen;
            return true;
        }
    }

    // Navigazione Prompt di risposta da tastiera (no wrap, come in main originale)
    if(state.responsePromptActive){
        if(const auto* kp = e.getIf<sf::Event::KeyPressed>()){
            if(kp->code == M_KEY_UP || kp->code == M_KEY_LEFT){
                if(state.responseOptionsCount && state.responseOptionsCount()>0){ if(state.responseSelected>0) state.responseSelected--; }
                return true;
            }
            if(kp->code == M_KEY_DOWN || kp->code == M_KEY_RIGHT){
                if(state.responseOptionsCount && state.responseOptionsCount()>0){ if(state.responseSelected + 1 < state.responseOptionsCount()) state.responseSelected++; }
                return true;
            }
            if(kp->code == M_KEY_ENTER){
                if(state.confirmResponsePrompt && (!state.responseOptionsCount || state.responseOptionsCount()>0)){
                    state.confirmResponsePrompt(state.responseSelected);
                }
                return true;
            }
            if(kp->code == M_KEY_ESCAPE){
                if(state.closeResponsePrompt) state.closeResponsePrompt();
                return true;
            }
        }
    }

    // Uscita dalla deck selection (ESC / Right click -> fade out)
    if(state.gamestate == GameState::DeckSelection){
        if(const auto* kp2 = e.getIf<sf::Event::KeyPressed>()){
            if(kp2->code == M_KEY_ESCAPE){
                if(!state.deckSelectionScreen.isFading()) state.deckSelectionScreen.startFadeOut(AppConfig::Timing::FadeOutFastSec);
                return true;
            }
        }
        if(isRight(e)){
            if(!state.deckSelectionScreen.isFading()) state.deckSelectionScreen.startFadeOut(AppConfig::Timing::FadeOutFastSec);
            return true;
        }
    }

    // In gioco: SPACE apre il popup "Torna alla Home" + tasti debug
    if(const auto* kp3 = e.getIf<sf::Event::KeyPressed>()){
    if(kp3->code == M_KEY_SPACE && state.gamestate == GameState::Playing){
            state.returnPopupActive = true;
            return true;
        }
        // Se si preme E mentre si è in drag ma non esiste una scelta Evoca/Posiziona attiva,
        // riportiamo la carta in mano (evitiamo che rimanga bloccata al centro)
        if(kp3->code == M_KEY_E && state.gamestate == GameState::Playing && !state.inputBlocked()){
            if(!state.chooseSummonOrSet && !state.pendingHandIndexForPlay.has_value()){
                if(state.cancelActiveDrag) state.cancelActiveDrag();
                return true;
            }
        }
        // Tasti fase/azioni debug centralizzati
        if(state.gamestate == GameState::Playing && !state.inputBlocked()){
            if(kp3->code == M_KEY_N){
                if(Game* game = state.getGame ? state.getGame() : nullptr){
                    game->advancePhase();
                    // apertura prompt risposta se utile
                    if(state.openResponsePromptIfAny) state.openResponsePromptIfAny();
                    // Reset selezione tributi fuori dalle Main
                    if(game->getTurn().getPhase() != GamePhase::Main1 && game->getTurn().getPhase() != GamePhase::Main2){
                        state.selectingTributes = false; state.selectedTributes.clear(); state.tributesNeeded = 0;
                    }
                    // UI battaglia
                    state.attackSelectionActive = (game->getTurn().getPhase() == GamePhase::Battle);
                    if(!state.attackSelectionActive){ state.selectedAttackerIndex.reset(); }
                    return true;
                }
            }
            if(kp3->code == M_KEY_T){
                if(Game* game = state.getGame ? state.getGame() : nullptr){
                    // Se non siamo in End, vai direttamente a End Phase; altrimenti prova a chiudere il turno
                    if(game->getTurn().getPhase() != GamePhase::End){
                        game->fastForwardToEndPhase();
                    } else if(!game->shouldAutoEndTurn() && !game->hasPendingSpecialSummon()) {
                        game->endTurn();
                    }
                    return true;
                }
            }
            if(kp3->code == M_KEY_O){
                if(Game* game = state.getGame ? state.getGame() : nullptr){
                    // Aggiungi un mostro dummy al campo avversario per testing
                    if(state.cardBackTexture){
                        Card dbg("Enemy Dummy", "Test", 1000, 1000,
                            sf::Vector2f(0.f,0.f), sf::Vector2f(0.f,0.f),
                            *state.cardBackTexture,
                            Type::Monster, Attribute::None, 4, {});
                        game->debugAddMonsterToOpponent(dbg);
                        if(state.syncMonsterZoneToField) state.syncMonsterZoneToField();
                    }
                    return true;
                }
            }
        }
        // Hotkey: Attacco diretto (A) durante la Battle Phase
    if(kp3->code == M_KEY_A && state.gamestate == GameState::Playing && !state.inputBlocked()){
            Game* game = state.getGame ? state.getGame() : nullptr;
            if(game && game->getTurn().getPhase() == GamePhase::Battle && state.selectedAttackerIndex.has_value()){
                if(game->canDeclareAttack(state.selectedAttackerIndex.value(), std::nullopt)){
                    if(game->declareAttack(state.selectedAttackerIndex.value(), std::nullopt)){
                        state.battleFeedbackMsg = "Attacco diretto";
                        state.battleFeedbackClock.restart();
                    }
                } else {
                    state.battleFeedbackMsg = "Attacco non consentito";
                    state.battleFeedbackClock.restart();
                }
                return true;
            }
        }
        // Gestione C/X e delle scelte E/P, A/S (spostata dal main)
    if(const auto* kp4 = e.getIf<sf::Event::KeyPressed>()){
            Game* game = state.getGame ? state.getGame() : nullptr;
            // Toggle posizione (C)
            if(kp4->code == M_KEY_C && state.gamestate == GameState::Playing && game && !state.inputBlocked() && !state.chooseSummonOrSet){
                if(state.selectedCardIsOnField && state.selectedCardIndex.has_value()){
                    if(game->togglePosition(state.selectedCardIndex.value())){
                        state.battleFeedbackMsg = "Posizione cambiata";
                        state.battleFeedbackClock.restart();
                    }
                }
                return true;
            }

            // Attiva M/T settata sotto al mouse (X)
            if(kp4->code == M_KEY_X && state.gamestate == GameState::Playing && game && !state.inputBlocked()){
                sf::Vector2i mpos = sf::Mouse::getPosition(state.window);
                bool triggered = false;
                int cur = game->getTurn().getCurrentPlayerIndex();
                if(cur == 0){
                    for(size_t i=0;i<state.p1STCards.size();++i){
                        if(state.p1STCards[i].getGlobalBounds().contains(sf::Vector2f((float)mpos.x, (float)mpos.y))){
                            if(game->activateSetSpellTrap(i)){
                                state.battleFeedbackMsg = "Carta attivata"; state.battleFeedbackClock.restart();
                                if(state.syncSpellTrapZones) state.syncSpellTrapZones();
                            } else {
                                state.battleFeedbackMsg = "Impossibile attivarla ora"; state.battleFeedbackClock.restart();
                            }
                            triggered = true; break;
                        }
                    }
                }
                return true;
            }

            // Scelte Evocazione/Posizionamento (E/P)
            if(state.chooseSummonOrSet && game && state.pendingHandIndexForPlay.has_value() && !state.inputBlocked()){
                if(kp4->code == M_KEY_E){
                    // Prova a evocare (fallback senza slot specifico)
                    size_t handIdx = state.pendingHandIndexForPlay.value();
                    std::cout << "[InputController] E pressed: tryNormalSummon handIdx=" << handIdx << std::endl;
                    bool ok = false;
                    try {
                        ok = game->tryNormalSummon(handIdx);
                    } catch(const std::exception &ex){
                        std::cout << "[InputController] exception in tryNormalSummon: " << ex.what() << std::endl;
                    } catch(...){
                        std::cout << "[InputController] unknown exception in tryNormalSummon" << std::endl;
                    }
                    std::cout << "[InputController] tryNormalSummon returned=" << ok << std::endl;
                    if(ok){ if(state.syncMonsterZoneToField) state.syncMonsterZoneToField(); }
                    else { state.battleFeedbackMsg = "Evocazione non riuscita"; state.battleFeedbackClock.restart(); if(state.restoreCardAtHandIndex) state.restoreCardAtHandIndex(handIdx); }
                    state.chooseSummonOrSet = false; state.pendingHandIndexForPlay.reset();
                    return true;
                } else if(kp4->code == M_KEY_P){
                    bool ok = game->tryNormalSet(state.pendingHandIndexForPlay.value());
                    if(ok){ if(state.syncMonsterZoneToField) state.syncMonsterZoneToField(); }
                    else { state.battleFeedbackMsg = "Posizionamento non riuscito"; state.battleFeedbackClock.restart(); if(state.restoreCardAtHandIndex) state.restoreCardAtHandIndex(state.pendingHandIndexForPlay.value()); }
                    state.chooseSummonOrSet = false; state.pendingHandIndexForPlay.reset();
                    return true;
                }
            }

            // Scelte Attiva/Set per Magie/Trappole (A/S)
            if(state.chooseActivateOrSetST && game && state.pendingHandIndexForPlay.has_value() && !state.inputBlocked()){
                if(kp4->code == M_KEY_A){
                    // Attiva la carta pendente dalla mano
                    size_t handIdx = state.pendingHandIndexForPlay.value();
                    if(game->activateSpellFromHand(handIdx)){
                        state.battleFeedbackMsg = "Carta attivata"; state.battleFeedbackClock.restart();
                        if(state.syncSpellTrapZones) state.syncSpellTrapZones();
                        if(state.updateHandLayout) state.updateHandLayout();
                    } else {
                        state.battleFeedbackMsg = "Impossibile attivarla ora"; state.battleFeedbackClock.restart();
                        if(state.restoreCardAtHandIndex) state.restoreCardAtHandIndex(handIdx);
                    }
                    state.chooseActivateOrSetST = false; state.pendingHandIndexForPlay.reset(); state.pendingIsSpellTrap = false;
                    return true;
                } else if(kp4->code == M_KEY_S){
                    // Distinguere tra Set di Magie/Trappole e Set di Mostri.
                    // Se la pending card è una ST, usare Game::setSpellOrTrap, altrimenti tryNormalSet per i mostri.
                    bool ok = false;
                    if(state.pendingIsSpellTrap){
                        ok = game->setSpellOrTrap(state.pendingHandIndexForPlay.value());
                        if(ok){
                            if(state.syncSpellTrapZones) state.syncSpellTrapZones();
                        } else {
                            state.battleFeedbackMsg = "Set non riuscito"; state.battleFeedbackClock.restart();
                            if(state.restoreCardAtHandIndex) state.restoreCardAtHandIndex(state.pendingHandIndexForPlay.value());
                        }
                    } else {
                        if(game->tryNormalSet(state.pendingHandIndexForPlay.value())){
                            ok = true;
                            if(state.syncMonsterZoneToField) state.syncMonsterZoneToField();
                        } else {
                            state.battleFeedbackMsg = "Set non riuscito"; state.battleFeedbackClock.restart();
                            if(state.restoreCardAtHandIndex) state.restoreCardAtHandIndex(state.pendingHandIndexForPlay.value());
                        }
                    }
                    state.chooseActivateOrSetST = false; state.pendingHandIndexForPlay.reset(); state.pendingIsSpellTrap = false;
                    return true;
                }
            }
        }
    }

    // ESC: gestione globale di chiusura/cancel
    if(const auto* kpEsc = e.getIf<sf::Event::KeyPressed>()){
        if(kpEsc->code == M_KEY_ESCAPE){
            // Chiudi solo il prompt di risposta (non fare continue qui di proposito)
            if(state.responsePromptActive){
                if(state.closeResponsePrompt) state.closeResponsePrompt();
                // non return, lascia che altre logiche di ESC facciano effetto
            }
            // ESC chiude overlay scelta DeckSend
            if(state.deckSendChoiceActive){
                if(Game* g = state.getGame ? state.getGame() : nullptr){ g->cancelPendingSendFromDeck(); }
                state.deckSendChoiceActive = false; state.deckSendCandidates.clear(); state.deckSendSelected = 0; state.deckSendOwner = -1;
                if(state.cancelActiveDrag) state.cancelActiveDrag();
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
                    // restore any active drag
                    if(state.cancelActiveDrag) state.cancelActiveDrag();
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
    sf::FloatRect gyRectTop(state.graveyardSlotPosTop, state.slotSize);
        if(const auto* mbp = e.getIf<sf::Event::MouseButtonPressed>()){if(mbp->button == M_MOUSE_LEFT){
                // Avvia hold sugli slot se il mouse è dentro
                state.extraOverlay.startHold(mousePosF, extraRect, state.gamestate);
                // Decide quale slot GY è sotto il mouse (bottom/top) e recupera le cards del relativo owner
                int gyOwner = -1; // 0 = bottom (P1), 1 = top (P2)
                if(gyRect.contains(mousePosF)) gyOwner = 0;
                else if(gyRectTop.contains(mousePosF)) gyOwner = 1;
                bool gyHasCards = false;
                if(state.getGame && gyOwner >= 0){ 
                    if(Game* g = state.getGame()) gyHasCards = !g->getGraveyardOf(gyOwner).empty(); 
                }
                // Start hold with the proper slot bounds (pass top/bottom accordingly)
                state.graveyardOverlay.startHold(mousePosF, (gyOwner==1)?gyRectTop:gyRect, state.gamestate, gyHasCards, gyOwner>=0?gyOwner:0);
                // Click per aprire/chiudere lista carte overlay
                state.extraOverlay.handleMousePress(mousePosF, state.getExtraCards ? state.getExtraCards() : std::vector<Card>{});
                // For the graveyard overlay press we must pass the correct owner's graveyard cards
                if(state.getGame){ 
                    std::vector<Card> gyCards;
                    if(gyOwner >= 0){
                        if(Game* g = state.getGame()) gyCards = g->getGraveyardOf(gyOwner);
                    } else {
                        gyCards = state.getGraveyardCards ? state.getGraveyardCards() : std::vector<Card>{};
                    }
                    state.graveyardOverlay.handleMousePress(mousePosF, gyCards);
                }
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
            if(kp->code == M_KEY_ENTER){
                if(state.confirmDeckSend) state.confirmDeckSend(state.deckSendSelected);
                return true;
            }
            if(kp->code == M_KEY_LEFT || kp->code == M_KEY_UP){
                if(!state.deckSendCandidates.empty()){
                    size_t cols = AppConfig::Ui::DeckSendCols;
                    if(kp->code == M_KEY_LEFT){
                        if(state.deckSendSelected>0) state.deckSendSelected--;
                    } else { // Up
                        if(state.deckSendSelected >= cols) state.deckSendSelected -= cols;
                    }
                }
                return true;
            }
            if(kp->code == M_KEY_RIGHT || kp->code == M_KEY_DOWN){
                if(!state.deckSendCandidates.empty()){
                    size_t cols = AppConfig::Ui::DeckSendCols;
                    if(kp->code == M_KEY_RIGHT){
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
