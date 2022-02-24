#include "App1.h"

int App1::init()
{
    Shader shader("shaders/vertexApp1.glsl", "shaders/fragmentApp1.glsl");
    shaderProgram = shader.ID;

    // Set up vertex data (and buffer(s)) and attribute pointers

    mesh = read_mesh("tetra.3do");
    mesh.generate_buffers();

    // Square

    return 0;
}

int App1::render()
{
    // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
    glfwPollEvents();

    // Render
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT); 

    glm::mat4 trans = glm::mat4(1.0);
    trans = glm::rotate(trans, (float)glfwGetTime(), glm::normalize(glm::vec3(0.5, 0.5, 0.0)));

    if (key_state(GLFW_KEY_W))
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, glm::value_ptr(trans));

    mesh.draw();

    // Display calculated framebuffer (back to front) and prepares displayed framebuffer to be drawn (front to back)
    glfwSwapBuffers(window);

    return 0;
}

int App1::quit()
{

    return 0;
}
