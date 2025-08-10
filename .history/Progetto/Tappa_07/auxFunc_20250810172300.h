#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <sstream>
#include <algorithm>
#include "Card/Card.h"
#include "Field/Field.h"

//-------------------------------- Funzioni di manipolazione del testo --------------------------------//

/**
 * Funzione per il word wrapping del testo che preserva la formattazione
 */
std::string wrapText(const std::string& text, const sf::Font& font, unsigned int characterSize, float maxWidth);

//-------------------------------- Funzioni di calcolo del layout --------------------------------//

/**
 * Calcola le dimensioni ottimali per gli slot del campo di gioco
 * @param windowSize Dimensioni della finestra
 * @param widthPercentage Percentuale della larghezza da utilizzare (default: 0.72f)
 * @param maxHeightPercentage Percentuale massima dell'altezza (default: 0.16f)
 * @param minHeight Altezza minima in pixel (default: 70.0f)
 * @return Dimensioni calcolate per gli slot
 */
sf::Vector2f calculateSlotSize(const sf::Vector2u& windowSize, 
                              float widthPercentage = 0.72f, 
                              float maxHeightPercentage = 0.16f, 
                              float minHeight = 70.0f);

/**
 * Calcola la spaziatura tra gli slot
 * @param windowSize Dimensioni della finestra
 * @param spacingPercentage Percentuale della larghezza per la spaziatura (default: 0.01f)
 * @return Spaziatura calcolata
 */
float calculateSpacing(const sf::Vector2u& windowSize, float spacingPercentage = 0.01f);

/**
 * Calcola l'offset delle zone dal centro
 * @param windowSize Dimensioni della finestra
 * @param slotSize Dimensioni degli slot
 * @param spacing Spaziatura tra slot
 * @param offsetPercentage Percentuale di offset (default: 0.06f)
 * @return Offset calcolato
 */
float calculateZoneOffset(const sf::Vector2u& windowSize, 
                         const sf::Vector2f& slotSize, 
                         float spacing, 
                         float offsetPercentage = 0.06f);

/**
 * Calcola i margini per le zone speciali
 * @param windowSize Dimensioni della finestra
 * @param marginPercentage Percentuale del margine (default: 0.04f)
 * @param minMargin Margine minimo in pixel (default: 20.0f)
 * @return Margine calcolato
 */
float calculateMargin(const sf::Vector2u& windowSize, 
                     float marginPercentage = 0.04f, 
                     float minMargin = 20.0f);

//-------------------------------- Funzioni per l'interfaccia utente delle carte --------------------------------//

/**
 * Mostra i dettagli di una carta in un pannello scrollabile
 * @param window Finestra di rendering
 * @param card Carta di cui mostrare i dettagli
 * @param font Font da utilizzare per il testo
 * @param panelPos Posizione del pannello
 * @param panelSize Dimensioni del pannello
 * @param scrollOffset Offset di scroll per il testo
 */
void showCardDetails(sf::RenderWindow& window, 
                    const Card& card, 
                    const sf::Font& font,
                    sf::Vector2f panelPos,
                    sf::Vector2f panelSize,
                    float scrollOffset);

//-------------------------------- Funzioni per disegnare il testo della schermata di avvio --------------------------------//

/**
 * Disegna la schermata di avvio con le istruzioni del gioco
 */
void drawStartScreen(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2u& windowSize);

//-------------------------------- Funzioni per la gestione delle carte in mano --------------------------------//

/**
 * Aggiorna le posizioni e dimensioni delle carte in mano per adattarle dinamicamente
 * @param cards Vettore delle carte in mano da aggiornare
 * @param windowSize Dimensioni della finestra
 * @param cardSize Dimensioni base delle carte
 * @param spacing Spaziatura base tra le carte
 * @param y Posizione Y delle carte
 * @param maxHandSize Numero massimo di carte visualizzabili senza ridimensionamento
 */
void updateHandPositions(std::vector<Card>& cards, 
                        const sf::Vector2u& windowSize,
                        const sf::Vector2f& cardSize,
                        float spacing,
                        float y,
                        int maxHandSize = 7);

//-------------------------------- Funzione per calcolare la posizione finale post animazione --------------------------------//

sf::Vector2f setHandPos(std::vector<Card>& cards, 
                        Card& card, 
                        sf::Vector2u& windowSize, 
                        sf::Vector2f& cardSize, 
                        float spacing, float y, 
                        int HAND_MAXSIZE);

//-------------------------------- Funzioni per le zone di drop del dragging --------------------------------//

/**
 * Struttura per rappresentare una zona di drop valida
 */
struct DropZone {
    sf::Vector2f position;     // Posizione del centro della zona
    sf::Vector2f size;         // Dimensioni della zona
    Slot::Type slotType;       // Tipo di slot (Monster, SpellTrap, etc.)
    int player;                // Player che possiede lo slot (P1 o P2)
    
    DropZone(sf::Vector2f pos, sf::Vector2f sz, Slot::Type type, int playerOwner)
        : position(pos), size(sz), slotType(type), player(playerOwner) {}
        
    // Controlla se un punto è all'interno della zona
    bool contains(sf::Vector2f point) const {
        sf::Vector2f halfSize = size / 2.f;
        return (point.x >= position.x - halfSize.x && point.x <= position.x + halfSize.x &&
                point.y >= position.y - halfSize.y && point.y <= position.y + halfSize.y);
    }
};

/**
 * Crea tutte le zone di drop valide per il campo di gioco
 * @param field Riferimento al campo di gioco
 * @param slotSize Dimensioni degli slot
 * @return Vettore di tutte le zone di drop disponibili
 */
std::vector<DropZone> createDropZones(const Field& field, const sf::Vector2f& slotSize);

/**
 * Trova la zona di drop sotto la posizione del mouse
 * @param mousePos Posizione del mouse
 * @param dropZones Vettore delle zone di drop
 * @return Puntatore alla zona trovata, nullptr se nessuna zona
 */
const DropZone* findDropZone(sf::Vector2f mousePos, const std::vector<DropZone>& dropZones);

/**
 * Controlla se una carta può essere piazzata in una specifica zona di drop
 * @param card Carta da controllare
 * @param dropZone Zona di drop target
 * @return true se la carta può essere piazzata, false altrimenti
 */
bool canDropCard(const Card& card, const DropZone& dropZone);

/**
 * Anima il ritorno di una carta alla mano
 * @param card Carta da far tornare
 * @param originalPosition Posizione originale nella mano
 * @param deltaTime Tempo trascorso dall'ultimo frame
 * @param returnSpeed Velocità di ritorno
 * @return true se l'animazione è completata, false altrimenti
 */
bool animateCardReturn(Card& card, sf::Vector2f originalPosition, float deltaTime, float returnSpeed = 1000.0f);
