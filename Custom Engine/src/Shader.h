#pragma once

// From https://learnopengl.com/Getting-started/Shaders

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>

struct ShaderCode
{
    std::string vertex_code;
    std::string fragment_code;

    // compute, geometry, etc...
};

class Shader
{
public:
    unsigned int ID;

    Shader() : ID(0) {}
    Shader(const std::string& shader_path);
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    void use();

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, glm::mat4 value) const;
    void setVec3(const std::string& name, glm::vec3 value) const;

private:
    void compile_shaders(const ShaderCode& sc);
};

