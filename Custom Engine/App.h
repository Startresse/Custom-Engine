#pragma once

// std includes
#include <iostream>

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef unsigned int uint;

class App
{
public:
	App() : App(640, 480) {}
	App(uint window_width, uint window_height) :
		width(window_width),
		height(window_height),
		window(nullptr)
	{}

	int run();

protected:
	uint width;
	uint height;
	
	GLFWwindow* window;

	int init();
	int render();
	int quit();
};

