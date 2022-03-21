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

class AxesBase
{
public:
    AxesBase() : AxesBase(default_vertex_shader, default_fragment_shader) {}
    AxesBase(const std::string& vs, const std::string& fs) : vertex_shader(vs), fragment_shader(fs), vao(0), vbo(0) {};

    void draw(const glm::mat4& m);

    void toggle() { display = !display; }

    static const std::string default_vertex_shader;
    static const std::string default_fragment_shader;

    static constexpr float line_size = 1.f;

protected:

    bool display = true;

    void create_buffers();
    void draw_base();

    std::vector<Line> lines;
    std::string vertex_shader;
    std::string fragment_shader;

    unsigned int vao, vbo;
    Shader program;

};

class Axes : public AxesBase
{
public:
    Axes() : AxesBase("shaders/vertexAxes.glsl", "shaders/fragmentAxes.glsl")
    {
        lines = 
        {
            Line({0, 0, 0}, Direction::right, Color::grid_red),
            Line({0, 0, 0}, Direction::up, Color::grid_blue),
            Line({0, 0, 0}, Direction::forward, Color::grid_green),
        };
    }
};

class Grid : public AxesBase
{
public:

    Grid() : AxesBase("shaders/vertexAxes.glsl", "shaders/fragmentAxes.glsl") { setup_grid(); }

    static constexpr float line_size = 2.f;
    static constexpr int size = 10;

private:

    void setup_grid();

};
