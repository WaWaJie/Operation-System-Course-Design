#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include<imgui.h>
#include<imgui_impl_sdl.h>
#include<imgui_impl_sdlrenderer.h>

#include<thread>
#include<chrono>

#include"resources_manager.h"
#include"singleton.h"
#include"timer.h"

class Application : public Singleton<Application>
{ 
	friend class Singleton<Application>;
public:
	Application();
	~Application();

	int run(int argc, char** argv);

private:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	bool is_quit = false;

	Timer timer_update_title;
	float fps_realtime = 1.0/60.0f;
};
		