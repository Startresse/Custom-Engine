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

    float current_line_width;
    glGetFloatv(GL_LINE_WIDTH, &current_line_width);

    glLineWidth(Grid::line_size);

    program.setMat4("MVP", mvp);
    const GLsizei axes_size = 2 * static_cast<GLsizei>(axes.size());
    const GLsizei grid_size = 2 * static_cast<GLsizei>(grid.size());
    if (display_axes)
        glDrawArrays(GL_LINES, 0, axes_size);
    if (display_grid)
        glDrawArrays(GL_LINES, axes_size, grid_size);

    glLineWidth(current_line_width);

    glUseProgram(0);
    glBindVertexArray(0);
}

void Axes::setup_grid()
{
    grid.clear();

    Color color;
    glm::vec3 a, b;
    for (int i = -Grid::size; i <= Grid::size; ++i)
    {
        float i_f = static_cast<float>(i);
        float gs = static_cast<float>(Grid::size);

        // along width 
        color = i ? Color::grid_grey : Color::grid_red;
        a = i_f * Direction::forward + gs * Direction::left;
        b = i_f * Direction::forward + gs * Direction::right;
        grid.push_back(Line(a, b, color));

        // along depth
        color = i ? Color::grid_grey : Color::grid_green;
        a = i_f * Direction::right + gs * Direction::back;
        b = i_f * Direction::right + gs * Direction::forward;
        grid.push_back(Line(a, b, color));
    }

    // TODO reupdate buffer
}

