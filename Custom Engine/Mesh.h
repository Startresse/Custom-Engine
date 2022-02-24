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

    Mesh() : VAO(0), VBO(0), EBO(0), nb_vertices(0), nb_triangles(0) {};

    ~Mesh();

    uint VAO, VBO, EBO;

    size_t nb_vertices;
    size_t nb_triangles;

    std::vector<float> vertices;
    std::vector<uint> indices;

};

Mesh read_mesh(std::string filepath);
