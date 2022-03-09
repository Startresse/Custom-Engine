#include "App1.h"

int App1::init()
{
    // Axes
    Shader axes_shader("shaders/vertexAxes.glsl", "shaders/fragmentAxes.glsl");
    axes_program = axes_shader.ID;
    float axes_vertices[] =
    {
        0, 0, 0,
        1, 0, 0,
        0, 0, 0,
        0, 1, 0,
        0, 0, 0,
        0, 0, 1,
    };
    glGenVertexArrays(1, &axes_vao);
    glGenBuffers(1, &axes_vbo);
    glBindVertexArray(axes_vao);

    glBindBuffer(GL_ARRAY_BUFFER, axes_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), axes_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    // Meshes
    Shader shader("shaders/vertexApp1.glsl", "shaders/fragmentApp1.glsl");
    shader_program = shader.ID;

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

    // axes
    glUseProgram(axes_program);
    glUniformMatrix4fv(glGetUniformLocation(axes_program, "MVP"), 1, GL_FALSE, glm::value_ptr(mvp));
    glBindVertexArray(axes_vao);
    glDrawArrays(GL_LINES, 0, 6);

    // Models
    glUseProgram(shader_program);

    glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(trans));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

    for (auto& mesh : meshes)
        mesh.draw(shader_program);

    return 0;
}

int App1::quit()
{
    return 0;
}
