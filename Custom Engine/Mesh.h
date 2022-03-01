#pragma once

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

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

test_mesh read_mesh(std::string filepath);
