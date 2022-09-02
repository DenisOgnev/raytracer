#include <SFML/Graphics.hpp>

const std::string WINDOW_NAME = "Test";
const uint32_t WIDTH = 500;
const uint32_t HEIGHT = 500;
const sf::Vector2u WINDOW_SIZE(WIDTH, HEIGHT);

void put_pixel(sf::Uint8* pixels, uint32_t x, uint32_t y, sf::Color color)
{
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
    {
        throw std::runtime_error("Failed to put pixel.");
    }
    pixels[y * (WIDTH * 4) + (x * 4)] = color.r;
    pixels[y * (WIDTH * 4) + (x * 4) + 1] = color.g;
    pixels[y * (WIDTH * 4) + (x * 4) + 2] = color.b;
    pixels[y * (WIDTH * 4) + (x * 4) + 3] = 255;
}

void fill(sf::Uint8* pixels, sf::Color color)
{
    for (size_t i = 0; i < WIDTH * HEIGHT * 4; i+=4)
    {
        pixels[i] = color.r;
        pixels[i + 1] = color.g;
        pixels[i + 2] = color.b;
        pixels[i + 3] = 255;
    }
}

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

    fill(pixels, sf::Color::Black);
    put_pixel(pixels, 499, 499, sf::Color::White);

    texture.update(pixels);
    

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
            }
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}