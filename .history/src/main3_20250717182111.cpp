#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Circle Test");

    sf::CircleShape circle(100.f); // raggio 100 pixel
    circle.setFillColor(sf::Color::Blue);
    circle.setPosition(300.f, 200.f); // posizione nel window

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White); // sfondo bianco
        window.draw(circle);
        window.display();
    }

    return 0;
}