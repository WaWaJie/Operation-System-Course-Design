#pragma once

#include"timer.h"
#include"vector2.h"
#include<SDL.h>
#include<SDL_image.h>
#include<vector>
#include<functional>
#include<iostream>

class Animation
{
public:
	Animation() = default;
	~Animation() = default;

	Animation(SDL_Renderer* renderer, IMG_Animation* anim, const Vector2& pos)
	{
		rect = { 0,0,0,0 };
		position = pos;
		original_size.x = 60 * 2.0 * ratio, original_size.y = 55 * 2.0 * ratio;
		for (int i = 0; i < anim->count; i++)
			tex_list.push_back(SDL_CreateTextureFromSurface(renderer, anim->frames[i]));

		timer.set_on_timeout([&]()
			{
				current_idx++;
				if (current_idx >= tex_list.size())
				{
					if (is_loop)current_idx = 0;
					else current_idx = tex_list.size() - 1;
				}
			});
	}
	void set_interval(float val) { timer.set_wait_time(val); }
	void set_callback(std::function<void()> callback) { timer.set_on_timeout(callback); }
	void set_ratio(float val) { ratio = val; }
	void set_size(const Vector2& val) { original_size = val; }
	int get_cur_idx(){return current_idx;}
	
	void on_update(float delta)
	{
		timer.on_update(delta);
	}

	void on_render(SDL_Renderer* renderer)
	{
		rect.x = position.x, rect.y = position.y;
		rect.w = original_size.x * ratio;
		rect.h = original_size.y * ratio;

		SDL_RenderCopy(renderer, tex_list[current_idx], nullptr, &rect);
	}

private:
	Timer timer;
	Vector2 position;
	Vector2 original_size;
	std::vector<SDL_Texture*>tex_list;
	int current_idx = 0;
	bool is_loop = true;
	float ratio = 1.0f;
	SDL_Rect rect;

};