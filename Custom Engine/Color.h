#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Color : public glm::vec4
{
public:
    Color() : glm::vec4(0.f, 0.f, 0.f, 1.f) {}

    explicit Color(const float value) : glm::vec4(value, value, value, 1.f) {}
    explicit Color(const float r, const float g, const float b, const float a = 1.f)
        : glm::vec4(r, g, b, a) {}

    Color(const Color& color, const float alpha) : glm::vec4(glm::vec3(color), alpha) {}

    float power() const;

    // Getter
    float r() const { return x; }
    float g() const { return y; }
    float b() const { return z; }
    float a() const { return w; }

    // Setter
    float& r() { return x; }
    float& g() { return y; }
    float& b() { return z; }
    float& a() { return w; }

    // Default colors
    static const Color black;
    static const Color white;

    static const Color red;
    static const Color green;
    static const Color blue;

    static const Color yellow;
    static const Color pink;
    static const Color cyan;

    // App specific colors
    static const Color grid_red;
    static const Color grid_green;
    static const Color grid_blue;
    static const Color grid_grey;

    static const Color background_grey;
};

