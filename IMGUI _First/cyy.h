#pragma once

#include"example.h"
#include"example_manager.h"
#include"singleton.h"
#include"animation.h"
#include<imgui.h>
#include<SDL.h>
#include<string>


class CYY :public Example
{
public:
	CYY(SDL_Renderer* renderer);
	~CYY();

	void on_enter() override;
	void on_exit() override;
	void on_update(float delta) override;
	void on_render(SDL_Renderer* renderer)override;
	
private:
	int choose_id = 0;
	SDL_Renderer* renderer;

	std::vector<Animation*>anim_list;
	SDL_Texture* texture_target;
};

