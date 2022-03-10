#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "App.h"
#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Axes.h"

/// <summary>
/// Exemple from https://learnopengl.com/ to check for tests and dev.
/// </summary>
class App1 : public App
{
public:
    using App::App;

    int init();
    int render();
    int quit();

    Axes default_axes;

private:
    Shader program;

    std::vector<Mesh> meshes;
};

