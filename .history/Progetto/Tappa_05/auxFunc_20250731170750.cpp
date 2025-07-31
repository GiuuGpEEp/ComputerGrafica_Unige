#include "auxFunc.h"


//-------------------------------- Funzioni per l'interfaccia utente delle carte --------------------------------//

void showCardDetails(sf::RenderWindow& window, 
                    const Card& card, 
                    const sf::Font& font,
                    sf::Vector2f panelPos,
                    sf::Vector2f panelSize,
                    float scrollOffset) {
    
    // Valori della carta
    auto [atk, def] = card.getValues();

    // Pannello dei dettagli
    sf::RectangleShape detailPanel(panelSize);
    detailPanel.setFillColor(sf::Color(0, 0, 0, 200));
    detailPanel.setOutlineColor(sf::Color::White);
    detailPanel.setOutlineThickness(2.f);
    detailPanel.setPosition(panelPos);

    // Creo la stringa con il testo principale 
    std::string detailRaw = "DETTAGLI CARTA\n\n";
    detailRaw += "Nome: " + card.getName() + "\n\n";
    detailRaw += "Descrizione:\n" + card.getDescription() + "\n\n";
    detailRaw += "ATK: " + std::to_string(atk) + "\n";
    detailRaw += "DEF: " + std::to_string(def);

    // Applica word wrapping al testo principale
    float textMaxWidth = panelSize.x - 40.f; // Margine di 20px per lato
    std::string wrappedText = wrapText(detailRaw, font, 14, textMaxWidth);

    // Partendo dalla stringa originale, dopo averne fatto il wrapping, creo l'effettivo oggetto Text (il testo scrollabile effettivo)
    sf::Text detailTextObj(font, wrappedText, 14);
    detailTextObj.setFillColor(sf::Color::White);
    detailTextObj.setPosition(sf::Vector2f(panelPos.x + 10.f, panelPos.y + 10.f - scrollOffset));

    // Crea il testo ESC fisso in fondo al pannello
    sf::Text escTextObj(font, "Premi ESC per chiudere", 12);
    escTextObj.setFillColor(sf::Color::Yellow);
    escTextObj.setPosition(sf::Vector2f(panelPos.x + 10.f, panelPos.y + panelSize.y - 25.f));

    // Disegna prima il pannello 
    window.draw(detailPanel);

    // Salva la view corrente
    sf::View originalView = window.getView();
    
    // Crea una view limitata al pannello per il clipping del testo scrollabile
    // Riduciamo l'altezza per lasciare spazio al testo ESC
    sf::View textView;
    textView.setSize(sf::Vector2f(panelSize.x - 20.f, panelSize.y - 50.f)); // Più spazio in basso per ESC
    textView.setCenter(sf::Vector2f(panelPos.x + panelSize.x/2.f, panelPos.y + (panelSize.y - 50.f)/2.f + 10.f));
    textView.setViewport(sf::FloatRect(
        sf::Vector2f((panelPos.x + 10.f) / window.getSize().x, (panelPos.y + 10.f) / window.getSize().y),
        sf::Vector2f((panelSize.x - 20.f) / window.getSize().x, (panelSize.y - 50.f) / window.getSize().y)
    ));

    // Imposta la view per il testo scrollabile e disegnalo
    window.setView(textView);
    window.draw(detailTextObj);
    
    // Ripristina la view originale per disegnare il testo ESC fisso
    window.setView(originalView);
    window.draw(escTextObj);
}

//-------------------------------- Funzioni per disegnare il testo della schermata di avvio --------------------------------//

void drawStartScreen(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2u& windowSize) {
    // Titolo del gioco
    sf::Text titleText(font, "YU-GI-OH! FIELD DISPLAY", 36);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setStyle(sf::Text::Bold);
    
    // Centra il titolo orizzontalmente
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition(sf::Vector2f((windowSize.x - titleBounds.size.x) / 2.0f, 50.0f));
    
    // Istruzioni del gioco
    std::string instructions = 
        "ISTRUZIONI:\n\n"
        "• Clicca su 'Visualizza Campo' per vedere il campo di gioco\n"
        "• Nel campo, clicca sulle carte per vedere i dettagli\n"
        "• Usa la rotella del mouse per scorrere i dettagli lunghi\n"
        "• Premi ESC per chiudere i dettagli delle carte\n"
        "• Premi ESC nel campo per tornare al menu principale\n"
        "• Clicca sul mazzo per pescare una carta\n\n"
        "Premi INVIO per iniziare!";
    
    sf::Text instructionsText(font, instructions, 18);
    instructionsText.setFillColor(sf::Color::White);
    
    // Posiziona le istruzioni
    sf::FloatRect instrBounds = instructionsText.getLocalBounds();
    instructionsText.setPosition(sf::Vector2f((windowSize.x - instrBounds.size.x) / 2.0f, 150.0f));
    
    // Disegna tutto
    window.draw(titleText);
    window.draw(instructionsText);
}
