#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "App.h"
#include "Shader.h"

/// <summary>
/// Exemple from https://learnopengl.com/ to check for tests and dev.
/// </summary>
class App1 : public App
{
public:
	using App::App;

	int init();
	int render();
	int quit();

private:
    GLuint VBO, VAO, EBO;
	GLuint shaderProgram;

};

