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

