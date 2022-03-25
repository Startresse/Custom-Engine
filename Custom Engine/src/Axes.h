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
#include "Camera.h"
#include "App.h"

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

/// <summary>
/// Automates all GL parts of grid display.
/// Just inherit this class and create a draw function which calk pre-draw and post draw
/// to setup shaders uniforms. (see Grid::draw or Axes::draw)
/// </summary>
class AxesBase
{
public:
    AxesBase() : AxesBase(default_vertex_shader, default_fragment_shader) {}
    AxesBase(const std::string& vs, const std::string& fs) : vertex_shader(vs), fragment_shader(fs), vao(0), vbo(0) {};

    // Keybind this to toggle or untoggle in App keycallbacks
    void toggle() { display = !display; }

    static const std::string default_vertex_shader;
    static const std::string default_fragment_shader;

    virtual float line_size() { return 1.f; };

protected:

    bool display = true;

    void create_buffers();
    void pre_draw();
    void post_draw();

    std::vector<Line> lines;
    std::string vertex_shader;
    std::string fragment_shader;

    unsigned int vao, vbo;
    Shader program;

private:

    float current_line_width = 0.f;

};

class Axes : public AxesBase
{
public:
    Axes() : AxesBase("src/shaders/vertexAxes.glsl", "src/shaders/fragmentAxes.glsl")
    {
        lines =
        {
            Line({0, 0, 0}, Direction::right, Color::grid_red),
            Line({0, 0, 0}, Direction::up, Color::grid_blue),
            Line({0, 0, 0}, Direction::forward, Color::grid_green),
        };
    }

    void draw(const Camera& camera, const App& app);
    float line_size() { return 2.f; };

    static constexpr float axes_pixel_size = 35;
};

class Grid : public AxesBase
{
public:

    Grid();

    void draw(const glm::mat4& m);

    float line_size() { return 2.f; };
    static constexpr int size = 10;
};
