#include "Axes.h"

void Axes::create_buffers()
{
    program = Shader("shaders/vertexAxes.glsl", "shaders/fragmentAxes.glsl");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Line) * (axes.size() + grid.size()), nullptr, GL_STATIC_DRAW);

    // put in update buffers
    GLsizei offset = 0;
    GLsizei sub_buffer_size = static_cast<GLsizei>(sizeof(Line) * axes.size());
    glBufferSubData(GL_ARRAY_BUFFER, offset, sub_buffer_size, axes.data());

    offset += sub_buffer_size;
    sub_buffer_size = static_cast<GLsizei>(sizeof(Line) * grid.size());
    glBufferSubData(GL_ARRAY_BUFFER, offset, sub_buffer_size, grid.data());

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Axes::draw(const glm::mat4& mvp)
{
    if (!vao)
        create_buffers();

    assert(vao);

    glBindVertexArray(vao);
    program.use();

    glLineWidth(grid_line_size);

    program.setMat4("MVP", mvp);
    GLsizei offset = 0;
    const GLsizei axes_size = 2 * static_cast<GLsizei>(axes.size());
    const GLsizei grid_size = 2 * static_cast<GLsizei>(grid.size());
    if (display_axes)
        glDrawArrays(GL_LINES, offset, axes_size);
    offset += axes_size;
    if (display_grid)
        glDrawArrays(GL_LINES, offset, grid_size);

    glLineWidth(default_line_size);

    glUseProgram(0);
    glBindVertexArray(0);
}

void Axes::setup_grid()
{
    grid.clear();

    Color color;
    for (int i = -grid_size; i <= grid_size; ++i)
    {
        // along X lines
        color = i ? grid_grey() : grid_red();
        grid.push_back(Line({ -grid_size, 0, i }, { grid_size, 0, i }, color));

        // along Z lines
        color = i ? grid_grey() : grid_green();
        grid.push_back(Line({ i, 0, -grid_size }, { i, 0, grid_size }, color));
    }

    // TODO reupdate buffer
}

