#include <SFML/Graphics.hpp>
#include <optional>

int main() {
    // SFML 3 richiede un Vector2u come dimensione
    sf::RenderWindow window(sf::VideoMode({640u, 480u}), "SFML 3 Static Test");

    sf::CircleShape circle(50.0f);
    circle.setFillColor(sf::Color::Green);
    circle.setPosition({200.f, 150.f});

    while (window.isOpen()) {
        // SFML 3 usa std::optional per gli eventi
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color::Black);
        window.draw(circle);
        window.display();
    }

    return 0;
}
