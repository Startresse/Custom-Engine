#pragma once

// std includes
#include <iostream>

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"

typedef GLFWwindow* Window;

/// <summary>
/// Virtual class that takes care of creating an OpenGL context and window.
/// Override the three main fonctions to create an OpenGL application.
/// </summary>
class App
{
public:
    App() : App(default_width, default_height) {}
    App(unsigned int window_width, unsigned int window_height)
        : App(window_width, window_height, default_major, default_minor) {}

    App(unsigned int window_width, unsigned int window_height, int major_gl_version, int minor_gl_version) :
        window(nullptr)
    {
        window = create_window(window_width, window_height, major_gl_version, minor_gl_version);
    }

    ~App();

    int run();


    // TODO protected ?
    Camera camera;

    int window_width();
    int window_height();

    static constexpr unsigned int default_width = 640;
    static constexpr unsigned int default_height = 480;

    static constexpr int default_major = 4;
    static constexpr int default_minor = 6;

protected:
    Window window;
    int width;
    int height;

    /// <summary>
    /// Used first and once at the run() function call.
    /// </summary>
    /// <returns>>=0 if it worked, some other integer otherwise.</returns>
    virtual int init() = 0;

    /// <summary>
    /// Used in the loop of the run() function.
    /// Each frame is a call to render().
    /// </summary>
    /// <returns>>=0 if it worked, some other integer otherwise.</returns>
    virtual int render() = 0;

    /// <summary>
    /// Used run() function.
    /// Each frame is a call to render().
    /// </summary>
    /// <returns>>=0 if it worked, some other integer otherwise.</returns>
    virtual int quit() = 0;

    virtual int handle_input() = 0;

    Window create_window(int width, int height, int major, int minor);

};

