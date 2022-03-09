#pragma once

#include <vector>

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

struct Line
{
    Line() : Line({ 0, 0, 0 }, { 0, 0, 0 }) {}
    Line(glm::vec3 start, glm::vec3 end) : a(start), b(end) {}

    glm::vec3 a;
    glm::vec3 b;
};

class Axes
{
public:
    Axes() : vao(0), vbo(0) { setup_grid(); };

    void draw(const glm::mat4 &mvp);

    // will only work if set before first draw call
    void set_grid_size(int size) { grid_size = size; setup_grid(); }

private:
    bool display_axes = true;
    bool display_grid = true;

    void create_buffers();
    void setup_grid();

    const std::vector<Line> axes =
    {
        Line({0, 0, 0}, {1, 0, 0}),
        Line({0, 0, 0}, {0, 1, 0}),
        Line({0, 0, 0}, {0, 0, 1}),
    };
    std::vector<Line> grid;
    int grid_size = 10;

    unsigned int vao, vbo;
    Shader program;
};

