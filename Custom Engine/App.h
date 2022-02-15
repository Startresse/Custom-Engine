#pragma once

// std includes
#include <iostream>

// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef unsigned int uint;
typedef GLFWwindow* Window;

class App
{
public:
	App() : App(640, 480) {}
	App(uint window_width, uint window_height) : App(window_width, window_height, 4, 6) {}

	App(uint window_width, uint window_height, int major_gl_version, int minor_gl_version) :
		window(nullptr)
	{
		window = create_window(window_width, window_height, major_gl_version, minor_gl_version);
	}

	~App();

	int run();

protected:
	Window window;
	int width;
	int height;

	virtual int init() = 0;
	virtual int render() = 0;
	virtual int quit() = 0;

	int window_width();
	int window_height();

	Window create_window(int width, int height, int major, int minor);

};

