#include <iostream>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <limits>
#include <vector>
#include <optional>

const std::string WINDOW_NAME = "Test";
const int32_t WIDTH = 500;
const int32_t HEIGHT = 500;
const sf::Vector2u WINDOW_SIZE(WIDTH, HEIGHT);

struct Sphere
{
    glm::vec3 center;
    float radius;
    sf::Color color;
};

const std::vector<Sphere> spheres{{glm::vec3(0.0f, -1.0f, 3.0f), 1.0f, sf::Color::Blue}};


void put_pixel_start_from_zero(sf::Uint8* pixels, uint32_t x, uint32_t y, sf::Color color)
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

void put_pixel(sf::Uint8* pixels, int32_t x, int32_t y, sf::Color color)
{
    if (x > (WIDTH - 1) / 2 || x < -WIDTH / 2 || y > (HEIGHT - 1) / 2 || y < -HEIGHT / 2)
    {
        throw std::runtime_error("Failed to put pixel.");
    }
    uint32_t fixed_x = WIDTH / 2 + x;
    uint32_t fixed_y = (HEIGHT + 1) / 2 - (y + 1);
    
    pixels[fixed_y * (WIDTH * 4) + (fixed_x * 4)] = color.r;
    pixels[fixed_y * (WIDTH * 4) + (fixed_x * 4) + 1] = color.g;
    pixels[fixed_y * (WIDTH * 4) + (fixed_x * 4) + 2] = color.b;
    pixels[fixed_y * (WIDTH * 4) + (fixed_x * 4) + 3] = 255;
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

std::tuple<float, float> intersect_ray_sphere(glm::vec3 camera_pos, glm::vec3 direction, Sphere sphere)
{
    glm::vec3 C = sphere.center;
    float r = sphere.radius;
    glm::vec3 OC = camera_pos - C;

    float k1 = glm::dot(direction, direction);
    float k2 = 2 * glm::dot(OC, direction);
    float k3 = glm::dot(OC, OC) - r * r;

    float discriminant = k2 * k2 - 4 * k1 * k3;
    if (discriminant < 0)
    {
        return std::make_tuple(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
    }

    float t1 = (-k2 + std::sqrtf(discriminant)) / (2 * k1);
    float t2 = (-k2 - std::sqrtf(discriminant)) / (2 * k1);

    return std::make_tuple(t1, t2);
}

sf::Color trace_ray(glm::vec3 camera_pos, glm::vec3 direction, float t_min, float t_max)
{
    float closest_t = std::numeric_limits<float>::infinity();
    std::optional<Sphere> closest_sphere;
    for (const auto& sphere : spheres)
    {
        float t1, t2;
        std::tie(t1, t2) = intersect_ray_sphere(camera_pos, direction, sphere);
        if (t1 > t_min && t1 < t_max && t1 < closest_t)
        {
            closest_t = t1;
            closest_sphere = sphere;
        }
        if (t2 > t_min && t2 < t_max && t2 < closest_t)
        {
            closest_t = t2;
            closest_sphere = sphere;
        }
    }
    if (!closest_sphere)
    {
        return sf::Color::White;
    }

    return closest_sphere.value().color;
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
    put_pixel(pixels, 249, 249, sf::Color::White);

    texture.update(pixels);


    glm::vec3 camera_pos(0.0f, 0.0f, 0.0f);
    uint32_t viewport_width = 1;
    uint32_t viewport_height = 1;
    uint32_t plane_distance = 1;

    for (int32_t x = -WIDTH / 2; x < (WIDTH + 1) / 2; x++)
    {
        for (int32_t y = -HEIGHT / 2; y < (HEIGHT + 1) / 2; y++)
        {
            glm::vec3 direction(x * (viewport_width / static_cast<double>(WIDTH)), y * (viewport_height / static_cast<double>(HEIGHT)), plane_distance);
            sf::Color result_color = trace_ray(camera_pos, direction, 1.0f, std::numeric_limits<float>::infinity());
            put_pixel(pixels, x, y, result_color);
        }
    }
    
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