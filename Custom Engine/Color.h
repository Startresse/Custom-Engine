#pragma once

#include <glm/vec3.hpp>

struct Color
{
    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    explicit Color(const float r_, const float g_, const float b_, const float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
    explicit Color(const float value) : r(value), g(value), b(value), a(1.0f) {}
    explicit Color(const glm::vec3& v, const float a_ = 1.0f) : r(v.x), g(v.y), b(v.z), a(a_) {}

    float r, g, b, a;

    float power() const;
};


Color black();
Color white();
Color red();
Color green();
Color blue();
Color grid_red();
Color grid_green();
Color grid_blue();
Color grid_grey();
Color pink();
Color yellow();
Color cyan();
