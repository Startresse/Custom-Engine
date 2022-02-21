#pragma once

#include "App.h"


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
    GLuint VBO, VAO;
	GLuint shaderProgram;

};

