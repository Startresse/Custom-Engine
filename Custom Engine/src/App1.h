#pragma once

#include <numbers>

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
    /* CONSTRUCTORS */

    using App::App;


    /* APP OVERLOAD */

    int init();
    int render();
    int quit();


    /* APP1 SPECIFIC FUNCTIONS*/

    int handle_input();


    /* CALLBACK RELATIVE VALUES*/

    Grid grid;
    Axes axes;

    glm::dvec2 last_mouse_pos;

    enum class CameraMovement
    {
        Rotate,
        Translate,
    };
    CameraMovement camera_movement;

private:
    Shader program;

    std::vector<Mesh> meshes;

    double last_time = 0.f;
};

