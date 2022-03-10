#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Color : public glm::vec4
{
public:
    Color() : glm::vec4(0, 0, 0, 1) {}

    explicit Color(const float value) : glm::vec4(value, value, value, 1.0f) {}
    explicit Color(const float r_, const float g, const float b, const float a = 1.0f)
        : glm::vec4(r_, g, b, a) {}

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

    // Default color
    static Color black();
    static Color white();

    static Color red();
    static Color green();
    static Color blue();

    static Color yellow();
    static Color pink();
    static Color cyan();

    // App specific colors
    static Color grid_red();
    static Color grid_green();
    static Color grid_blue();
    static Color grid_grey();
};

