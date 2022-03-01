#include "Mesh.h"

#include <sstream>
#include <fstream>

test_mesh::~test_mesh()
{
    if (VAO)
        glDeleteVertexArrays(1, &VAO);
    if (VBO)
        glDeleteBuffers(1, &VBO);
    if (EBO)
        glDeleteBuffers(1, &EBO);
}

test_mesh read_mesh(std::string filepath)
{
    test_mesh mesh;

    std::ifstream file;

    file.open(filepath);
    if (!file.is_open()) {
        std::cout << "Couldn't open file" << std::endl;
        exit(0);
    }

    std::string line;

    // Read vertices
    while (std::getline(file, line))
    {

        if (line[0] == '#')
            continue;

        std::stringstream ss(line);
        float x, y, z;
        ss >> x >> y;
        if (!(ss >> z))
            break;

        mesh.vertices.push_back(x);
        mesh.vertices.push_back(y);
        mesh.vertices.push_back(z);

    }

    // Read color
    while (std::getline(file, line))
    {

        if (line[0] == '#')
            continue;

        std::stringstream ss(line);
        float x, y, z;
        ss >> x >> y;
        if (!(ss >> z))
            break;

        mesh.vertices.push_back(x);
        mesh.vertices.push_back(y);
        mesh.vertices.push_back(z);

    }

    // Read indices
    while (std::getline(file, line))
    {

        if (line[0] == '#')
            continue;

        int a, b, c;
        std::stringstream ss(line);
        ss >> a >> b;
        if (!(ss >> c))
            break;
        mesh.indices.push_back(a);
        mesh.indices.push_back(b);
        mesh.indices.push_back(c);

    }

    file.close();

    return mesh;
}

int test_mesh::generate_buffers()
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(sizeof(float) * vertices.size() / 2));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    return 0;
}

void test_mesh::draw()
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, nb_indices(), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
