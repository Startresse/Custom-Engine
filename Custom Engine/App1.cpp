#include "App1.h"

int App1::init()
{

    // Meshes
    program = Shader("shaders/vertexApp1.glsl", "shaders/fragmentApp1.glsl");

    meshes.push_back(read_mesh("data/cube.obj"));
    //meshes.push_back(read_mesh("data/cornell.obj"));

    // Camera
    camera.set_position(glm::vec3(0.0f, 2.0f, 3.0f));
    camera.set_target(glm::vec3(0.0f, 0.0f, 0.0f));

    return 0;
}

int App1::input()
{

    // Wireframe display
    if (key_state(GLFW_KEY_W))
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    return 0;
}

int App1::render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.set_position(glm::vec3(glm::rotate(glm::mat4(1.0), (float)glfwGetTime(), glm::vec3(0, 1, 0)) * glm::vec4(0.0f, 2.0f, 3.0f, 1.0f)));
    glm::mat4 view = camera.view();
    glm::mat4 proj = camera.projection();
    glm::mat4 mvp = proj * view * glm::mat4(1.0);

    glm::mat4 trans = glm::mat4(1.0);
    //trans = glm::rotate(trans, (float)glfwGetTime(), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
    //trans = glm::translate(trans, glm::vec3(-0.5, -0.5, 0.0f));
    //trans = glm::rotate(trans, 45.0f, glm::normalize(glm::vec3(0.5, 0.5, 0.0)));

    default_axes.draw(mvp);

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
