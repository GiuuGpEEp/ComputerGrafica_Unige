#pragma once
#include <SFML/Graphics.hpp>

// Compatibility macros to abstract SFML 2 vs SFML 3 enum scoping
#if defined(SFML_VERSION_MAJOR) && (SFML_VERSION_MAJOR >= 3)
#define M_MOUSE_LEFT sf::Mouse::Button::Left
#define M_MOUSE_RIGHT sf::Mouse::Button::Right
#define M_KEY_SPACE sf::Keyboard::Key::Space
#define M_KEY_ENTER sf::Keyboard::Key::Enter
#define M_KEY_ESCAPE sf::Keyboard::Key::Escape
#define M_KEY_UP sf::Keyboard::Key::Up
#define M_KEY_DOWN sf::Keyboard::Key::Down
#define M_KEY_LEFT sf::Keyboard::Key::Left
#define M_KEY_RIGHT sf::Keyboard::Key::Right
#define M_KEY_N sf::Keyboard::Key::N
#define M_KEY_Y sf::Keyboard::Key::Y
#define M_KEY_C sf::Keyboard::Key::C
#define M_KEY_X sf::Keyboard::Key::X
#define M_KEY_E sf::Keyboard::Key::E
#define M_KEY_P sf::Keyboard::Key::P
#define M_KEY_A sf::Keyboard::Key::A
#define M_KEY_D sf::Keyboard::Key::D
#define M_KEY_S sf::Keyboard::Key::S
#else
#define M_MOUSE_LEFT sf::Mouse::Left
#define M_MOUSE_RIGHT sf::Mouse::Right
#define M_KEY_SPACE sf::Keyboard::Space
#define M_KEY_ENTER sf::Keyboard::Enter
#define M_KEY_ESCAPE sf::Keyboard::Escape
#define M_KEY_UP sf::Keyboard::Up
#define M_KEY_DOWN sf::Keyboard::Down
#define M_KEY_LEFT sf::Keyboard::Left
#define M_KEY_RIGHT sf::Keyboard::Right
#define M_KEY_N sf::Keyboard::N
#define M_KEY_Y sf::Keyboard::Y
#define M_KEY_C sf::Keyboard::C
#define M_KEY_X sf::Keyboard::X
#define M_KEY_E sf::Keyboard::E
#define M_KEY_P sf::Keyboard::P
#define M_KEY_A sf::Keyboard::A
#define M_KEY_D sf::Keyboard::D
#define M_KEY_S sf::Keyboard::S
#endif
