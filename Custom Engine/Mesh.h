#pragma once

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include "Typedefs.h"

class Mesh
{
public:

    Mesh() : VAO(0), VBO(0), EBO(0) {};

    ~Mesh();

    int generate_buffers();

    void draw();

    uint VAO, VBO, EBO;

    size_t nb_vertices() { return indices.size(); }
    size_t nb_indices() { return indices.size(); }
    size_t nb_triangles() { return indices.size() / 3; }

    std::vector<float> vertices;
    std::vector<uint> indices;

};

Mesh read_mesh(std::string filepath);
