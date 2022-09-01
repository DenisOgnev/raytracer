#include <SFML/Graphics.hpp>

const std::string WINDOW_NAME = "Test";
const unsigned int WIDTH = 500;
const unsigned int HEIGHT = 500;

int main()
{
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(WIDTH, HEIGHT)), WINDOW_NAME);

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;
}