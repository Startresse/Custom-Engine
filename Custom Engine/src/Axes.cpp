#include "Axes.h"


/* AXES BASE */

const std::string AxesBase::default_shader = "src/shaders/grid.glsl";

void AxesBase::create_buffers()
{
    if (!shader.empty())
        program = Shader(shader);
    else
        program = Shader(vertex_shader, fragment_shader);

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

void AxesBase::pre_draw()
{
    if (!vao)
        create_buffers();

    assert(vao);

    glBindVertexArray(vao);
    program.use();

    glGetFloatv(GL_LINE_WIDTH, &current_line_width);

    glLineWidth(line_size());
}

void AxesBase::post_draw()
{
    if (display)
        glDrawArrays(GL_LINES, 0, 2 * static_cast<GLsizei>(lines.size()));

    glLineWidth(current_line_width);

    glUseProgram(0);
    glBindVertexArray(0);
}


/* AXES */

void Axes::draw(const Camera& camera, const App& app)
{
    pre_draw();

    glm::mat4 p = glm::mat4(1.f);
    p = glm::translate(p, camera.position() - camera.direction());
    p = glm::scale(p, glm::vec3(axes_pixel_size / app.window_height()));
    glm::mat4 mvp = camera.projection() * camera.view() * p;

    program.setMat4("MVP", mvp);
    program.setInt("window_width", app.window_width());
    program.setInt("window_height", app.window_height());
    post_draw();
}


/* GRID */

Grid::Grid() : AxesBase("src/shaders/grid.glsl")
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

}

void Grid::draw(const glm::mat4& m)
{
    pre_draw();
    program.setMat4("MVP", m);
    post_draw();
}
