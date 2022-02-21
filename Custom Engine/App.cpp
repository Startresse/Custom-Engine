#include "App.h"

// Warning : called on every frame but displayed only on realese...
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    return window;
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

    glViewport(0, 0, window_width(), window_height());

    while (!glfwWindowShouldClose(window))
    {
        render();
    }

    if(quit() < 0)
		return -1;

	return 0;
}

App::~App()
{
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
}

