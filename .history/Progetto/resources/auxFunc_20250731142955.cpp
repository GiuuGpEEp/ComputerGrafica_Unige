#include <sstream>
#include <SFML/Graphics.hpp>
#include "auxFunc.h"
#include <algorithm>


//-------------------------------- Funzione per il word wrapping del testo che preserva la formattazione --------------------------------//

std::string wrapText(const std::string& text, const sf::Font& font, unsigned int characterSize, float maxWidth) {
    std::string wrappedText;
    std::istringstream lines(text);
    std::string line;
    
    sf::Text tempText(font, "", characterSize);
    
    // Processa riga per riga per preservare la formattazione originale
    while (std::getline(lines, line)) {
        if (line.empty()) {
            // Preserva le righe vuote
            wrappedText += "\n";
            continue;
        }
        
        std::istringstream words(line);
        std::string word;
        std::string currentLine;
        
        while (words >> word) {
            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
            tempText.setString(testLine);
            
            if (tempText.getLocalBounds().size.x <= maxWidth) {
                currentLine = testLine;
            } else {
                if (!currentLine.empty()) {
                    wrappedText += currentLine + "\n";
                    currentLine = word;
                } else {
                    // Parola troppo lunga per una riga
                    wrappedText += word + "\n";
                }
            }
        }
        
        if (!currentLine.empty()) {
            wrappedText += currentLine + "\n";
        }
    }
    
    return wrappedText;
}

//-------------------------------- Funzioni di calcolo del layout --------------------------------//

sf::Vector2f calculateSlotSize(const sf::Vector2u& windowSize, 
                              float widthPercentage, 
                              float maxHeightPercentage, 
                              float minHeight) {
    // Calcola dimensioni slot in base alla finestra
    float availableWidth = windowSize.x * widthPercentage;
    float slotWidth = availableWidth / 5.0f; // 3 slot + 2 spaziature equivalenti
    
    // Mantieni proporzioni 128:180 dalla texture originale  
    float aspectRatio = 180.0f / 128.0f;
    float slotHeight = slotWidth * aspectRatio;
    
    // Limita l'altezza se troppo grande
    float maxHeight = windowSize.y * maxHeightPercentage;
    if (slotHeight > maxHeight) {
        slotHeight = maxHeight;
        slotWidth = slotHeight / aspectRatio;
    }
    
    // Assicurati che non sia troppo piccolo
    if (slotHeight < minHeight) {
        slotHeight = minHeight;
        slotWidth = slotHeight / aspectRatio;
    }
    
    return sf::Vector2f(slotWidth, slotHeight);
}

float calculateSpacing(const sf::Vector2u& windowSize, float spacingPercentage) {
    // Spaziatura proporzionale alla dimensione della finestra
    return windowSize.x * spacingPercentage;
}

float calculateZoneOffset(const sf::Vector2u& windowSize, 
                         const sf::Vector2f& slotSize, 
                         float spacing, 
                         float offsetPercentage) {
    // Calcola offset dinamico per evitare overflow e lasciare spazio per le carte
    float maxTotalHeight = 4 * slotSize.y + 3 * spacing; // 2 righe per giocatore + spaziature
    return std::min(windowSize.y * offsetPercentage, (windowSize.y - maxTotalHeight) / 5.0f);
}

float calculateMargin(const sf::Vector2u& windowSize, 
                     float marginPercentage, 
                     float minMargin) {
    // Margine dinamico dai bordi
    return std::max(windowSize.x * marginPercentage, minMargin);
}

//-------------------------------- Funzioni per disegnare il testo della schermata di avvio --------------------------------//
 