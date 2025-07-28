////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>

#include <vector>
#include <string>

int main()
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    auto window = sf::RenderWindow(sf::VideoMode({800u, 600u}), "CMake SFML Project", sf::Style::Default, sf::State::Windowed, settings);

    window.setFramerateLimit(100);
    sf::Vector2f offset({0.f, 0.f});
    auto rot = sf::Angle::Zero;
    while (window.isOpen()) // main loop
    {
        while (const std::optional event = window.pollEvent())
        { // event loop + hevent handler
            if (event->is<sf::Event::Closed>())
            {
                window.close();
                break;
            }
        }
        // Do graphics
        window.clear();

        sf::RectangleShape r({300.f, 200.f});
        r.setOrigin({150.f, 100.f});
        r.setPosition({100.f, 100.f});
        r.move(offset);
        r.rotate(rot);
        r.setFillColor(sf::Color::Green);
        window.draw(r);
        offset += {1.f, 1.f};
        rot += sf::degrees(1.f);
        window.display();
    }
}
