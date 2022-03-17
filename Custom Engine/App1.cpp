#include "App1.h"

// Keyboard input
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    App1* app = static_cast<App1*>(glfwGetWindowUserPointer(window));

    // G + 1 : toggle axes
    if (key == GLFW_KEY_1 && action == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_G))
        app->default_axes.toggle_axes_display();

    // G + 2 : toggle grid
    if (key == GLFW_KEY_2 && action == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_G))
        app->default_axes.toggle_grid_display();

    // W : toggle wireframe
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        GLint polygon_mode;
        glGetIntegerv(GL_POLYGON_MODE, &polygon_mode);
        if (polygon_mode == GL_LINE)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}

// Scroll wheel input
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    App1* app = static_cast<App1*>(glfwGetWindowUserPointer(window));
    
    // Scroll : zoom (forward) / unzoom (bakwards)
    app->camera.zoom(-yoffset);
}

int App1::init()
{
    // Set input callbacks
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    else
        std::cout << "WARNING : mouse raw motion not supported" << std::endl;

    glfwSetKeyCallback(window, key_callback);
    //glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);


    // Shader
    program = Shader("shaders/vertexApp1.glsl", "shaders/fragmentApp1.glsl");

    // Meshes
    meshes.push_back(read_mesh("data/cube.obj"));
    //meshes.push_back(read_mesh("data/cornell.obj"));

    // Camera
    camera.set_target(glm::vec3(0.f, 0.f, 0.f));
    glm::vec4 camera_pos_init_h = glm::vec4(Direction::up + 3.f * Direction::forward, 1.f);
    camera.set_position(camera_pos_init_h);

    return 0;
}

int App1::render()
{
    //double fps = 1.f / (glfwGetTime() - last_time);
    //last_time = glfwGetTime();
    //std::cout << fps << " fps" << std::endl;

    Color c = Color::background_grey;
    glClearColor(c.x, c.y, c.z, c.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glm::vec4 camera_pos_init_h = glm::vec4(Direction::up + 3.f * Direction::forward, 1.f);
    //camera.set_position(glm::vec3(glm::rotate(glm::mat4(1.0), (float)glfwGetTime(), Direction::up) * camera_pos_init_h));
    //camera.translate(glm::vec3(0.005f, 0.f, 0.f));
    //camera.zoom(sin(glfwGetTime()) >= 0.f);
    camera.rotate_around_target(glm::radians(90.f)/ 144.f, glm::vec3(1, 0.1, 0));
    //camera.rotate_around_position(glm::radians(90.f)/ 144.f, glm::vec3(1, 0.1, 0));

    glm::mat4 view = camera.view();
    glm::mat4 proj = camera.projection();
    glm::mat4 mvp = proj * view * glm::mat4(1.0);

    glm::mat4 trans = glm::mat4(1.0);
    //trans = glm::rotate(trans, (float)glfwGetTime(), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
    //trans = glm::translate(trans, glm::vec3(-0.5, -0.5, 0.0f));
    //trans = glm::rotate(trans, 45.0f, glm::normalize(glm::vec3(0.5, 0.5, 0.0)));

    default_axes.draw(mvp);
    // grid won't display over anything
    //glClear(GL_DEPTH_BUFFER_BIT);

    // Models
    program.use();
    program.setMat4("model", trans);
    program.setMat4("view", view);
    program.setMat4("projection", proj);

    for (auto& mesh : meshes)
        mesh.draw(program.ID);

    return 0;
}

int App1::quit()
{
    return 0;
}
