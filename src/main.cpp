#include <SFML/Graphics.hpp>

const std::string WINDOW_NAME = "Test";
const uint32_t WIDTH = 500;
const uint32_t HEIGHT = 500;
const sf::Vector2u WINDOW_SIZE(WIDTH, HEIGHT);

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE), WINDOW_NAME);

    sf::Uint8* pixels = new sf::Uint8[WIDTH * HEIGHT * 4];
    sf::Texture texture;
    if (!texture.create(WINDOW_SIZE))
    {
        throw std::runtime_error("Failed to create texture.");
    }
    sf::Sprite sprite(texture);

    for (size_t i = 0; i < WIDTH * HEIGHT * 4; i+=4)
    {
        pixels[i] = 100;
        pixels[i + 1] = 100;
        pixels[i + 2] = 100;
        pixels[i + 3] = 255;
    }

    texture.update(pixels);
    

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}