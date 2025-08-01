#pragma once
#include <SFML/Graphics.hpp>
#include <string>


//-------------------------------- Funzioni di utilità per il testo --------------------------------//

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
 * Disegna la schermata di avvio
 */
void drawStartScreen(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2u& windowSize);