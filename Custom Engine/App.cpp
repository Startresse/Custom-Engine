#include "App.h"

// Warning : called on every frame but displayed only on realese...
void update_framebuffer(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    App* app = (App*) glfwGetWindowUserPointer(window);

    constexpr float fov = glm::radians(45.f);
    float aspect_ratio = static_cast<float>(app->window_width()) / static_cast<float>(app->window_height());
    float near_plane = 0.1f;
    float far_plane = 100.f;
    app->camera.set_projection(glm::perspective(fov, aspect_ratio, near_plane, far_plane));
}

const std::string window_name = "Main Window";

Window App::create_window(int w, int h, int major, int minor)
{
    // Init GLFW
    glfwInit();

    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    Window window = glfwCreateWindow(w, h, window_name.c_str(), nullptr, nullptr);

    // Create a GLFWwindow object that we can use for GLFW's functions
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    glfwGetFramebufferSize(window, &width, &height);

    // use GLEW modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return nullptr;
    }

    // Set window size change callback
    glfwSetFramebufferSizeCallback(window, update_framebuffer);
    glfwSetWindowUserPointer(window, (void*)this);

    return window;
}

int App::key_state(int key_code)
{
    return glfwGetKey(window, key_code);
}

int App::window_width()
{
    glfwGetFramebufferSize(window, &width, &height);
    return width;
}

int App::window_height()
{
    glfwGetFramebufferSize(window, &width, &height);
    return height;
}

int App::run()
{
    if (init() < 0)
        return -1;

    // Use update framebuffer function to set camera proj and glViewport
    update_framebuffer(window, window_width(), window_height());

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        render();

        // Display calculated framebuffer (back to front) and prepares displayed framebuffer to be drawn (front to back)
        glfwSwapBuffers(window);
    }

    if (quit() < 0)
        return -1;

    return 0;
}

App::~App()
{
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
}

