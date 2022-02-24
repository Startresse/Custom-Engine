#include "Mesh.h"

Mesh::~Mesh()
{
    if (VAO)
        glDeleteVertexArrays(1, &VAO);
    if (VBO)
        glDeleteBuffers(1, &VBO);
    if (EBO)
        glDeleteBuffers(1, &EBO);
}

Mesh read_mesh(std::string filepath)
{
    Mesh mesh;

    /// Triangle
    //mesh.vertices =
    //{
    //    // positions            // colors
    //    -0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,         // Left
    //     0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,         // Right
    //     0.0f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,         // Top
    //};
    //mesh.indices =
    //{
    //    0, 1, 2,
    //};

    // Square
    mesh.vertices =
    {
        // positions            // colors
         0.5f,  0.5f, 0.0f,     1.0f, 0.0f, 0.0f,         // Top Right
         0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,         // Bottomt Right
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,         // Bottomt Left
        -0.5f,  0.5f, 0.0f,     1.0f, 0.0f, 1.0f,         // Top Left
    };
    mesh.indices =
    {
        0, 1, 3,    // First triangle
        1, 2, 3,    // Second triangle
    };


    //// Tetra
    //mesh.vertices =
    //{
    //    // positions            // colors
    //    -0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,
    //     0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,
    //     0.0f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,
    //     0.0f,  0.0f, 0.5f,     0.0f, 0.0f, 0.0f,
    //};
    //mesh.indices =
    //{
    //    0, 1, 2,
    //    0, 1, 3,
    //    1, 2, 3,
    //    0, 2, 3,
    //};

    return mesh;
}

int Mesh::generate_buffers()
{
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    return 0;
}

void Mesh::draw()
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, nb_indices(), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
