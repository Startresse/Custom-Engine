#include "Mesh.h"

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

Mesh read_mesh(std::string filepath)
{
    Mesh mesh;

    return mesh;
}
