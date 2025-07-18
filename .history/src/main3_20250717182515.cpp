#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <optional> // importante per std::optional

int main() {
    // Uso corretto di sf::VideoMode con sf::Vector2u
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Circle Test");

    sf::CircleShape circle(50.f); // raggio
    circle.setFillColor(sf::Color::Green);
    circle.setPosition({300.f, 200.f}); // Usa un vettore

    while (window.isOpen()) {
        // Nuovo modo di gestire gli eventi con std::optional
        while (auto event = window.pollEvent()) {
            if (event->type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        window.draw(circle);
        window.display();
    }

    return 0;
}
