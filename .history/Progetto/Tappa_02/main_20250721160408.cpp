////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>

#include <vector>
#include <string>
#include <math.h>
#include <iostream>

int main()
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    auto window = sf::RenderWindow(sf::VideoMode({800u, 600u}), "CMake SFML Project", sf::Style::Default, sf::State::Windowed, settings);
    window.setFramerateLimit(10);
    sf::Texture texture("../resources/SpaceInvaders.png");
    bool swap = false;
    sf::Vector2f pos({0.f, 0.f}), step({5.f, 0.f});
    int sign = 1;

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

        sf::Sprite sprite(texture);
        sprite.scale({10.f, 10.f});
        sprite.setColor(sf::Color::Red);
        sprite.setPosition(pos);
        unsigned sw = sprite.getLocalBounds().size.x;
        if (swap)
            sprite.setTextureRect(sf::IntRect({5, 1}, {8, 8}));
        else
            sprite.setTextureRect(sf::IntRect({5, 11}, {8, 8}));
        swap = !swap;
        if ((pos.x >= window.getSize().x - 80 && sign == 1) || (pos.x <= 0 && sign == -1))
        {
            pos += {0.f, 30.f};
            sign = -sign;
        }
        else
            pos += {step.x * sign, step.y};
        window.draw(sprite);

        window.display();
    }
}
