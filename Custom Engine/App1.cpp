#include "App1.h"

int App1::init()
{
    Shader shader("shaders/vertexApp1.glsl", "shaders/fragmentApp1.glsl");
    shaderProgram = shader.ID;

    // Set up vertex data (and buffer(s)) and attribute pointers

    //mesh = old_read_mesh("tetra.3do");
    //mesh.generate_buffers();

    mesh = read_mesh("data/cube.obj");

    // Square

    return 0;
}

int App1::render()
{
    // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
    glfwPollEvents();

    // TODO move
    glEnable(GL_DEPTH_TEST);  

    if (key_state(GLFW_KEY_W))
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Render
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 camera_dir = glm::normalize(camera_pos - camera_target);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 camera_right = glm::normalize(glm::cross(up, camera_dir));
    glm::vec3 camera_up = glm::cross(camera_dir, camera_right);

    glm::mat4 view;
    view = glm::lookAt(camera_pos, camera_target, camera_up);

    glm::mat4 trans = glm::mat4(1.0);
    trans = glm::rotate(trans, (float)glfwGetTime(), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
    trans = glm::translate(trans, glm::vec3(-0.5, -0.5, 0.0f));
    trans = glm::rotate(trans, 45.0f, glm::normalize(glm::vec3(0.5, 0.5, 0.0)));

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), static_cast<float>(window_width()) / window_height(), 0.1f, 100.0f);
    glUseProgram(shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(trans));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

    //glm::vec3 light_pos = camera_pos;
    //glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "light_pos"), 1, GL_FALSE, glm::value_ptr(light_pos));

    mesh.draw(shaderProgram);

    // Display calculated framebuffer (back to front) and prepares displayed framebuffer to be drawn (front to back)
    glfwSwapBuffers(window);

    return 0;
}

int App1::quit()
{

    return 0;
}
