#pragma once

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>

#include <iostream>
#include <vector>

class Mesh
{
public:
    Mesh() : positions(0), normals(0), texcoords(0), indices(0), vao(0), buffer(0), index_buffer(0) {}
    ~Mesh();

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    std::vector<unsigned int> indices;

    void draw(GLuint program);

private:
    unsigned int vao, buffer, index_buffer;

    unsigned int create_buffers();

};

Mesh read_mesh(std::string filename);

class test_mesh
{
public:

    test_mesh() : VAO(0), VBO(0), EBO(0) {};

    ~test_mesh();

    int generate_buffers();

    void draw();

    unsigned int VAO, VBO, EBO;

    size_t nb_vertices() { return indices.size(); }
    size_t nb_indices() { return indices.size(); }
    size_t nb_triangles() { return indices.size() / 3; }

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

};

test_mesh old_read_mesh(std::string filepath);
