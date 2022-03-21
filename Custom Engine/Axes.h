#pragma once

#include <vector>

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Color.h"
#include "Direction.h"

namespace GridParam
{
    constexpr int size = 10;
    constexpr float line_size = 2.0f;
}

struct Point
{
    Point() : Point(glm::vec3(0.0f, 0.0f, 0.0f)) {}
    Point(glm::vec3 p) : Point(p, Color::white) {}
    Point(glm::vec3 p, Color c) : coord(p), color(c) {}

    glm::vec3 coord;
    Color color;
};

struct Line
{
    Line() : Line({ 0, 0, 0 }, { 0, 0, 0 }) {}
    Line(glm::vec3 start, glm::vec3 end) : Line(start, end, Color::white) {}
    Line(glm::vec3 start, glm::vec3 end, Color color) : a(start, color), b(end, color) {}

    Point a;
    Point b;
};

class Grid
{
public:
    Grid() : vao(0), vbo(0) { setup_grid(); };

    void draw(const glm::mat4& mvp);

    void toggle() { display = !display; }

private:

    bool display = true;

    void create_buffers();
    void setup_grid();

    std::vector<Line> grid;

    unsigned int vao, vbo;
    Shader program;
};

class Axes
{
public:
    Axes() : vao(0), vbo(0) {}

    void draw(const glm::mat4& view);

    void toggle() { display = !display; }

private:
    bool display = true;

    void create_buffers();

    const std::vector<Line> axes =
    {
        Line({0, 0, 0}, Direction::right, Color::grid_red),
        Line({0, 0, 0}, Direction::up, Color::grid_blue),
        Line({0, 0, 0}, Direction::forward, Color::grid_green),
    };

    unsigned int vao, vbo;
    Shader program;
};
