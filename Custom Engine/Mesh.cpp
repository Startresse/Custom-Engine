#include "Mesh.h"

#include <sstream>
#include <fstream>

unsigned int Mesh::create_buffers()
{
    if (positions.size() == 0)
        return 0;

    if (vao)
        return vao;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    size_t index_buffer_size;
    index_buffer_size = sizeof(unsigned int) * indices.size();
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, indices.data(), GL_STATIC_DRAW);

    size_t buffer_size;
    buffer_size = sizeof(glm::vec3) * positions.size();
    // TODO update depending on normals, etc...

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_STATIC_DRAW);

    // TODO add other stuff
    size_t offset = 0;
    size_t size = sizeof(glm::vec3) * positions.size();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, positions.data());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *) offset);
    glEnableVertexAttribArray(0);

    //offset += size;
    //size = normals.size();
    //glBufferSubData(GL_ARRAY_BUFFER, offset, size, normals.data());
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *) offset);
    //glEnableVertexAttribArray(1);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vao;
}

void Mesh::draw(GLuint program)
{
    if (!vao)
        create_buffers();

    assert(vao);

    if (!program)
    {
        std::cout << "[error] (can't draw) : no program" << std::endl;
        return;
    }

    glUseProgram(program);

    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

Mesh::~Mesh()
{
    if (vao)
        glDeleteVertexArrays(1, &vao);
    if (buffer)
        glDeleteBuffers(1, &buffer);
    if (index_buffer)
        glDeleteBuffers(1, &index_buffer);
}

Mesh read_mesh(std::string filename)
{
    Mesh mesh;

    mesh.positions = {
        glm::vec3( 0.5f,  0.5f, 0.0f),
        glm::vec3(-0.5f,  0.5f, 0.0f),
        glm::vec3(-0.5f, -0.5f, 0.0f),
        glm::vec3( 0.5f, -0.5f, 0.0f),
    };
    mesh.indices = { 0, 1, 2, 0, 2, 3 };

    return mesh;
}
