#include <iostream>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <limits>
#include <vector>
#include <optional>
#include <memory>
#include <omp.h>


const std::string WINDOW_NAME = "Raytracer";
const int32_t WIDTH = 500;
const int32_t HEIGHT = 500;

const glm::vec3 camera_position(0.0f, 0.0f, 0.0f);


struct Sphere
{
    glm::vec3 center;
    float radius;
    sf::Color color;
    int specular;
    float reflective;
    float radius_squared;
    glm::vec3 OC;
    float dot_oc;

    Sphere(glm::vec3 _center, float _radius, sf::Color _color, int _specular, float _reflective) : center(_center), radius(_radius), color(_color), specular(_specular), reflective(_reflective)
    {
        radius_squared = radius * radius;
        OC = camera_position - center;
        dot_oc = dot(OC, OC);
    }
};


struct Light
{
    enum Type
    {
        AMBIENT, POINT, DIRECTIONAL
    };

    Type type;
    float intensity;
    glm::vec3 position;
    glm::vec3 direction;
};


const std::vector<Sphere> Spheres
{
    {glm::vec3(0.0f, -1.0f, 3.0f), 1.0f, sf::Color::Red, 500, 0.2f},
    {glm::vec3(2.0f, 0.0f, 4.0f), 1.0f, sf::Color::Blue, 500, 0.3f},
    {glm::vec3(-2.0f, 0.0f, 4.0f), 1.0f, sf::Color::Green, 10, 0.4f},
    {glm::vec3(0.0f, -5001.0f, 0.0f), 5000.0f, sf::Color::Yellow, 1000, 0.5f}
};
const std::vector<Light> Lights
{
    {Light::AMBIENT, 0.2f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
    {Light::POINT, 0.6f, glm::vec3(2.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
    {Light::DIRECTIONAL, 0.2f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 4.0f, 4.0f)}
};


class RaytracerApp
{
public:
    RaytracerApp(std::string window_name, int32_t width, int32_t height) : WINDOW_NAME(window_name), WIDTH(width), HEIGHT(height), WINDOW_SIZE(sf::Vector2u(WIDTH, HEIGHT)), window(sf::RenderWindow(sf::VideoMode(WINDOW_SIZE), WINDOW_NAME)) 
    {
        pixels = new sf::Uint8[WIDTH * HEIGHT * 4];
        if (!texture.create(WINDOW_SIZE))
            throw std::runtime_error("Failed to create texture.");
        sprite = sf::Sprite(texture);
    }

    void run()
    {
        main_loop();
    }

    ~RaytracerApp()
    {
        delete [] pixels;
    }


private:
    const std::string WINDOW_NAME;
    const int32_t WIDTH;
    const int32_t HEIGHT;
    const sf::Vector2u WINDOW_SIZE;
    sf::RenderWindow window;

    sf::Uint8* pixels;
    sf::Texture texture;
    sf::Sprite sprite;

    float viewport_width = 1.0f;
    float viewport_height = 1.0f;
    float plane_distance = 1.0f;

    const int32_t recursion_depth = 3;

    sf::Clock clock;

    void main_loop()
    {
        while (window.isOpen())
        {
            float current_time = clock.restart().asSeconds();
            float fps = 1.0f / (current_time);

            std::cout << "frametime: " << current_time << ", fps: " << fps << "\n";

            #pragma omp parallel for
            for (int32_t x = -WIDTH / 2; x < (WIDTH + 1) / 2; x++)
            {
                for (int32_t y = -HEIGHT / 2; y < (HEIGHT + 1) / 2; y++)
                {
                    glm::vec3 direction(x * (viewport_width / static_cast<float>(WIDTH)), y * (viewport_height / static_cast<float>(HEIGHT)), plane_distance);
                    sf::Color result_color = trace_ray(camera_position, direction, 1.0f, std::numeric_limits<float>::infinity(), recursion_depth);
                    // #pragma omp critical
                    put_pixel(x, y, result_color);
                }
            }
            
            texture.update(pixels);
            

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
    }


    void put_pixel_start_from_zero(int32_t x, int32_t y, sf::Color color)
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


    void put_pixel(int32_t x, int32_t y, sf::Color color)
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


    void fill(sf::Color color)
    {
        for (size_t i = 0; i < WIDTH * HEIGHT * 4; i+=4)
        {
            pixels[i] = color.r;
            pixels[i + 1] = color.g;
            pixels[i + 2] = color.b;
            pixels[i + 3] = 255;
        }
    }


    int clamp(int value, int lowest, int highest)
    {
        if (value > highest)
        {
            return highest;
        }
        if (value < lowest)
        {
            return lowest;
        }
        return value;
    }
    

    sf::Color trace_ray(glm::vec3 camera_pos, glm::vec3 direction, float t_min, float t_max, int current_depth)
    {
        float closest_t;
        std::optional<Sphere> closest_sphere;
        std::tie(closest_sphere, closest_t) = closest_intersection(camera_pos, direction, t_min, t_max);
        if (!closest_sphere)
        {
            return sf::Color::Black;
        }

        glm::vec3 point = camera_pos + closest_t * direction;
        glm::vec3 normal = point - closest_sphere.value().center;
        normal = glm::normalize(normal);
        float lighting = compute_lighting(point, normal, -direction, closest_sphere.value().specular);

        sf::Color t_color = closest_sphere.value().color;
        sf::Color local_color;
        local_color.r = static_cast<sf::Uint8>(clamp(static_cast<int>(static_cast<float>(t_color.r) * lighting), 0, 255));
        local_color.g = static_cast<sf::Uint8>(clamp(static_cast<int>(static_cast<float>(t_color.g) * lighting), 0, 255));
        local_color.b = static_cast<sf::Uint8>(clamp(static_cast<int>(static_cast<float>(t_color.b) * lighting), 0, 255));

        float reflective = closest_sphere.value().reflective;
        if (recursion_depth <= 0 || reflective <= 0.0f)
        {
            return local_color;
        }

        glm::vec3 reflected_direction = reflect_ray(-direction, normal);
        sf::Color reflected_color = trace_ray(point, reflected_direction, 0.1f, std::numeric_limits<float>::infinity(), current_depth - 1);
        sf::Color result_color;
        result_color.r = static_cast<sf::Uint8>(clamp(static_cast<int>(static_cast<float>(local_color.r) * (1.0f - reflective) + static_cast<float>(reflected_color.r) * reflective), 0, 255));
        result_color.g = static_cast<sf::Uint8>(clamp(static_cast<int>(static_cast<float>(local_color.g) * (1.0f - reflective) + static_cast<float>(reflected_color.g) * reflective), 0, 255));
        result_color.b = static_cast<sf::Uint8>(clamp(static_cast<int>(static_cast<float>(local_color.b) * (1.0f - reflective) + static_cast<float>(reflected_color.b) * reflective), 0, 255));

        return result_color;
    }


    float compute_lighting(const glm::vec3 &point, const glm::vec3 &normal, const glm::vec3 &view, int specular)
    {
        float i = 0.0f;
        for (const auto &light : Lights)
        {
            if (light.type == Light::AMBIENT)
            {
                i += light.intensity;
            }
            else
            {
                glm::vec3 light_vector;
                float t_min = 0.1f, t_max = 0.0f;
                if (light.type == Light::POINT)
                {
                    light_vector = light.position - point;
                    t_max = 1;
                }
                else if (light.type == Light::DIRECTIONAL)
                {
                    light_vector = light.direction;
                    t_max = std::numeric_limits<float>::infinity();
                }

                float shadow_t;
                std::optional<Sphere> shadow_sphere;
                std::tie(shadow_sphere, shadow_t) = closest_intersection(point, light_vector, t_min, t_max);
                if (shadow_sphere.has_value())
                {
                    continue;
                }

                float n_dot_l = glm::dot(normal, light_vector);
                if (n_dot_l > 0.0f)
                {
                    i += light.intensity * n_dot_l / (glm::length(normal) * glm::length(light_vector));
                }

                if (specular != -1)
                {
                    glm::vec3 R = reflect_ray(light_vector, normal);
                    float r_dot_v = glm::dot(R, view);
                    if (r_dot_v > 0.0f)
                        i += light.intensity * std::powf((r_dot_v / (glm::length(R) * glm::length(view))), static_cast<float>(specular));
                }
            }
        }
        return i;
    }


    std::tuple<std::optional<Sphere>, float> closest_intersection(const glm::vec3 &camera_pos, const glm::vec3 &direction, float t_min, float t_max)
    {
        float closest_t = std::numeric_limits<float>::infinity();
        std::optional<Sphere> closest_sphere;
        float k1 = glm::dot(direction, direction);
        for (const auto& sphere : Spheres)
        {
            float t1, t2;
            std::tie(t1, t2) = intersect_ray_sphere(camera_pos, direction, sphere, k1);
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
        return std::make_tuple(closest_sphere, closest_t);
    }


    std::tuple<float, float> intersect_ray_sphere(const glm::vec3 &camera_pos, const glm::vec3 &direction, const Sphere &sphere, const float k1)
    {
        glm::vec3 center = sphere.center;
        float radius_squared = sphere.radius_squared;
        glm::vec3 OC = camera_pos - center;

        float k2 = 2 * glm::dot(OC, direction);
        float k3 = sphere.dot_oc - radius_squared;

        float discriminant = k2 * k2 - 4 * k1 * k3;
        if (discriminant < 0)
        {
            return std::make_tuple(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
        }

        float t1 = (-k2 + std::sqrtf(discriminant)) / (2 * k1);
        float t2 = (-k2 - std::sqrtf(discriminant)) / (2 * k1);

        return std::make_tuple(t1, t2);
    }


    glm::vec3 reflect_ray(glm::vec3 ray, glm::vec3 normal)
    {
        return 2.0f * normal * glm::dot(normal, ray) - ray;
    }
};


int main()
{
    RaytracerApp app(WINDOW_NAME, WIDTH, HEIGHT);
    try 
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}