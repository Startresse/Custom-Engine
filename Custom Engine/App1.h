#pragma once
#include "App.h"
class App1 : public App
{
public:
	using App::App;

	int init();
	int render();
	int quit();

};

