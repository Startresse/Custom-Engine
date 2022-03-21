#include "Axes.h"

void Axes::create_buffers()
{
    program = Shader("shaders/vertexAxes.glsl", "shaders/fragmentAxes.glsl");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLsizei buffer_size = static_cast<GLsizei>(sizeof(Line) * axes.size());
    glBufferData(GL_ARRAY_BUFFER, buffer_size, axes.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Axes::draw(const glm::mat4& view)
{
    if (!vao)
        create_buffers();

    assert(vao);

    glBindVertexArray(vao);
    program.use();

    float current_line_width;
    glGetFloatv(GL_LINE_WIDTH, &current_line_width);

    glLineWidth(GridParam::line_size); // TODO Change

    program.setMat4("MVP", view);
    if (display)
        glDrawArrays(GL_LINES, 0, 2 * static_cast<GLsizei>(axes.size()));

    glLineWidth(current_line_width);

    glUseProgram(0);
    glBindVertexArray(0);
}

void Grid::create_buffers()
{
    program = Shader("shaders/vertexAxes.glsl", "shaders/fragmentAxes.glsl");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Line) * grid.size(), grid.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Grid::draw(const glm::mat4& mvp)
{
    if (!vao)
        create_buffers();

    assert(vao);

    glBindVertexArray(vao);
    program.use();

    float current_line_width;
    glGetFloatv(GL_LINE_WIDTH, &current_line_width);

    glLineWidth(GridParam::line_size);

    program.setMat4("MVP", mvp);
    if (display)
        glDrawArrays(GL_LINES, 0, 2 * static_cast<GLsizei>(grid.size()));

    glLineWidth(current_line_width);

    glUseProgram(0);
    glBindVertexArray(0);
}

void Grid::setup_grid()
{
    grid.clear();

    Color color;
    glm::vec3 a, b;
    for (int i = -GridParam::size; i <= GridParam::size; ++i)
    {
        float i_f = static_cast<float>(i);
        float gs = static_cast<float>(GridParam::size);

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

