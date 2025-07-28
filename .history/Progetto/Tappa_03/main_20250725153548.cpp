////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>

#include <vector>
#include <string>
#include <math.h>

int main()
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    auto window = sf::RenderWindow(sf::VideoMode({800u, 600u}), "CMake SFML Project", sf::Style::Default, sf::State::Windowed, settings);
    window.setFramerateLimit(30);

    sf::Texture texture("../resources/1856-51_stone-texture-wall-charcoal-color.jpg");
    sf::Transform transform(200.f, 0.f, 300.f,
                            0.f, 200.f, 300.f,
                            0.f, 0.f, 1.f);
    sf::RenderStates states;
    states.transform = transform;
    states.texture = &texture;
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

        float sqrt32 = sqrt(3.f) / 2.f;
        sf::VertexArray wall(sf::PrimitiveType::TriangleFan, 8);
        wall[0].position = sf::Vector2f(0.f, 0.f);
        wall[1].position = sf::Vector2f(sqrt32, -0.5f);
        wall[2].position = sf::Vector2f(0.f, -1.f);
        wall[3].position = sf::Vector2f(-sqrt32, -0.5f);
        wall[4].position = sf::Vector2f(-sqrt32, 0.5f);
        wall[5].position = sf::Vector2f(0.f, 1.f);
        wall[6].position = sf::Vector2f(sqrt32, 0.5f);
        wall[7].position = sf::Vector2f(sqrt32, -0.5f);
        wall[0].color = wall[1].color = wall[2].color =
            wall[3].color = wall[4].color = wall[5].color =
                wall[6].color = wall[7].color = sf::Color::Green;
        wall[0].texCoords = {1500.f, 1000.f};
        wall[1].texCoords = {sqrt32 * 1000 + 1500, -0.5 * 1000 + 1000};
        wall[2].texCoords = {1500, -1000 + 1000};
        wall[3].texCoords = {-sqrt32 * 1000 + 1500, -0.5 * 1000 + 1000};
        wall[4].texCoords = {-sqrt32 * 1000 + 1500, 0.5 * 1000 + 1000};
        wall[5].texCoords = {1500, 1000 + 1000};
        wall[6].texCoords = {sqrt32 * 1000 + 1500, 0.5 * 1000 + 1000};
        wall[7].texCoords = {sqrt32 * 1000 + 1500, -0.5 * 1000 + 1000};

        window.draw(wall, states);
        window.display();
    }
}
