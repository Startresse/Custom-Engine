#include "Axes.h"

void Axes::create_buffers()
{
    program = Shader("shaders/vertexAxes.glsl", "shaders/fragmentAxes.glsl");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes[0]) * axes.size(), axes.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
}

void Axes::draw(const glm::mat4 &mvp)
{
    if (!vao)
        create_buffers();

    assert(vao);

    glBindVertexArray(vao);
    program.use();

    program.setMat4("MVP", mvp);
    glDrawArrays(GL_LINES, 0, 2 * static_cast<GLsizei>(axes.size()));

    glUseProgram(0);
    glBindVertexArray(0);
}

