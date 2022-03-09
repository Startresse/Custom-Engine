#pragma once

#include <vector>

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

class Axes
{
public:
    Axes() : Axes({ {{0, 0, 0}, {1, 0, 0}}, {{0, 0, 0}, {0, 1, 0}}, {{0, 0, 0}, {0, 0, 1}} }) {}
    Axes(std::vector<std::pair<glm::vec3, glm::vec3>> lines) : axes(lines), vao(0), vbo(0) {}

    void draw(const glm::mat4 &mvp);
private:
    void create_buffers();

    std::vector<std::pair<glm::vec3, glm::vec3>> axes;

    unsigned int vao, vbo;
    Shader program;
};

