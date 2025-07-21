#include "Card/Card.h"
#include <vector>

int main() {
    // Carico le texture delle carte
    sf::Texture textureFlipped(("../../Progetto/resources/Texture1.png"));
    sf::Texture textureNonFlipped(("../../Progetto/resources/CardNotSet.jpg"));

    // Creo le carte di test
    std::vector<Card> cards;
    cards.reserve(10);
    cards.emplace_back("Drago Bianco", 
        "Questo drago leggendario è una potente macchina distruttrice. Virtualmente invincibile, sono in pochi ad aver fronteggiato questa creatura ed essere sopravvissuti per raccontarlo.", 
        3000, 2500, sf::Vector2f(50, 50), textureFlipped);
    cards.emplace_back("Mago Nero", 
        "Il più potente tra i maghi per abilità offensive e difensive.", 
        2500, 2000, sf::Vector2f(200, 50), textureNonFlipped);

    // Finestra
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Progetto Tappa 02");

    std::optional<size_t> selectedCardIndex;

    // Font
    sf::Font detailFont;
    if (!detailFont.openFromFile("../../Progetto/resources/ITCKabelStdDemi.TTF")) {
        if (!detailFont.openFromFile("C:/Windows/Fonts/calibri.ttf")) {
            std::cerr << "ERRORE: Impossibile caricare nessun font!" << std::endl;
        }
    }

    // Variabili per la view scrollabile
    sf::View detailView;
    float scrollOffset = 0.f;

    int frameCount = 0;
    while (window.isOpen()) {
        frameCount++;

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Click
            if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButton->button == sf::Mouse::Button::Left) {
                    sf::Vector2i mousePos = mouseButton->position;
                    bool cardClicked = false;
                    for (size_t i = 0; i < cards.size(); ++i) {
                        if (cards[i].isClicked(mousePos)) {
                            selectedCardIndex = i;
                            scrollOffset = 0.f; // resetta lo scroll
                            cardClicked = true;
                            break;
                        }
                    }
                    if (!cardClicked) selectedCardIndex.reset();
                }
            }

            // ESC
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::escape) {
                    selectedCardIndex.reset();
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

        window.clear(sf::Color::Black);

        // Disegna le carte
        for (size_t i = 0; i < cards.size(); ++i) {
            cards[i].draw(window);
        }

        // Dettagli carta selezionata
        if (selectedCardIndex.has_value()) {
            const Card& selectedCard = cards[selectedCardIndex.value()];
            auto [atk, def] = selectedCard.getValues();

            // --- Pannello
            sf::Vector2f panelPos{400.f, 150.f};
            sf::Vector2f panelSize{300.f, 200.f};
            sf::RectangleShape detailPanel(panelSize);
            detailPanel.setFillColor(sf::Color(0, 0, 0, 200));
            detailPanel.setOutlineColor(sf::Color::White);
            detailPanel.setOutlineThickness(2.f);
            detailPanel.setPosition(panelPos);

            // --- Testo
            std::string detailRaw = "DETTAGLI CARTA\n\n";
            detailRaw += "Nome: " + selectedCard.getName() + "\n\n";
            detailRaw += "Descrizione:\n" + selectedCard.getDescription() + "\n\n";
            detailRaw += "ATK: " + std::to_string(atk) + "\n";
            detailRaw += "DEF: " + std::to_string(def) + "\n\n";
            detailRaw += "Premi ESC per chiudere";

            sf::Text detailText;
            detailText.setFont(detailFont);
            detailText.setCharacterSize(14);
            detailText.setFillColor(sf::Color::White);
            detailText.setPosition(panelPos.x + 10.f, panelPos.y + 10.f - scrollOffset);
            detailText.setString(detailRaw);

            // --- View limitata al pannello
            detailView.reset(sf::FloatRect(panelPos.x, panelPos.y, panelSize.x, panelSize.y));
            window.setView(detailView);

            window.draw(detailPanel);
            window.draw(detailText);

            // --- Reset view
            window.setView(window.getDefaultView());
        }

        window.display();
    }

    return 0;
}
