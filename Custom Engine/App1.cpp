#include "App1.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    App1* app = (App1*) glfwGetWindowUserPointer(window);

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

int App1::init()
{

    // Meshes
    program = Shader("shaders/vertexApp1.glsl", "shaders/fragmentApp1.glsl");

    meshes.push_back(read_mesh("data/cube.obj"));
    //meshes.push_back(read_mesh("data/cornell.obj"));

    // Camera
    camera.set_position(glm::vec3(0.0f, 2.0f, 3.0f));
    camera.set_target(glm::vec3(0.0f, 0.0f, 0.0f));

    glfwSetKeyCallback(window, key_callback);

    return 0;
}

int App1::render()
{
    glClearColor(0.212f, 0.212f, 0.212f, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.set_position(glm::vec3(glm::rotate(glm::mat4(1.0), (float)glfwGetTime(), glm::vec3(0, 1, 0)) * glm::vec4(0.0f, 1.0f, 3.0f, 1.0f)));
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
