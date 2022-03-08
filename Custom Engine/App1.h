#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "App.h"
#include "Shader.h"
#include "Mesh.h"

/// <summary>
/// Exemple from https://learnopengl.com/ to check for tests and dev.
/// </summary>
class App1 : public App
{
public:
    using App::App;

    int input();

    int init();
    int render();
    int quit();

private:
    GLuint shader_program;

    std::vector<Mesh> meshes;

    GLuint axes_program;
    GLuint axes_vao, axes_vbo;

};

