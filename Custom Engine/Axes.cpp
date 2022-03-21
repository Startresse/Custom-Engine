#include "Axes.h"

const std::string AxesBase::default_vertex_shader = "shaders/vertexAxes.glsl";
const std::string AxesBase::default_fragment_shader = "shaders/fragmentAxes.glsl";

void AxesBase::create_buffers()
{
    program = Shader(vertex_shader.c_str(), fragment_shader.c_str());

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLsizei buffer_size = static_cast<GLsizei>(sizeof(Line) * lines.size());
    glBufferData(GL_ARRAY_BUFFER, buffer_size, lines.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void AxesBase::draw(const glm::mat4& m)
{
    if (!vao)
        create_buffers();

    assert(vao);

    glBindVertexArray(vao);
    program.use();

    float current_line_width;
    glGetFloatv(GL_LINE_WIDTH, &current_line_width);

    glLineWidth(line_size);

    program.setMat4("MVP", m);
    if (display)
        glDrawArrays(GL_LINES, 0, 2 * static_cast<GLsizei>(lines.size()));

    glLineWidth(current_line_width);

    glUseProgram(0);
    glBindVertexArray(0);
}

void Grid::setup_grid()
{
    lines.clear();

    Color color;
    glm::vec3 a, b;
    for (int i = -size; i <= size; ++i)
    {
        float i_f = static_cast<float>(i);
        float gs = static_cast<float>(size);

        // along width 
        color = i ? Color::grid_grey : Color::grid_red;
        a = i_f * Direction::forward + gs * Direction::left;
        b = i_f * Direction::forward + gs * Direction::right;
        lines.push_back(Line(a, b, color));

        // along depth
        color = i ? Color::grid_grey : Color::grid_green;
        a = i_f * Direction::right + gs * Direction::back;
        b = i_f * Direction::right + gs * Direction::forward;
        lines.push_back(Line(a, b, color));
    }

    // TODO reupdate buffer
}

