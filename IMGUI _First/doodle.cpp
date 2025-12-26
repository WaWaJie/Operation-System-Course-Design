#include"doodle.h"
#include"resources_manager.h"
#include<random>
#include<iostream>
#include<SDL_mixer.h>

Doodle::Doodle(SDL_Renderer* renderer)
{
	this->renderer = renderer;
	texture_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 332, 296);
}
Doodle::~Doodle()
{
	SDL_DestroyTexture(texture_target);
}

void Doodle::on_enter()
{
	//随机数
	srand((unsigned int)time(NULL));

	Animation* animation = new Animation(renderer, ResourcesManager::instance()->find_animation(u8"xiduo"), { 0,0 });
	animation->set_interval(0.04f);
	animation->set_ratio(1.5f);
	animation->set_size({ 224,224 });

	anim_list.push_back(animation);
	//播放音乐chunk
	Mix_PlayChannel(-1, ResourcesManager::instance()->find_audio(u8"doodle_dance"), -1);
}
void Doodle::on_exit()
{
	for (auto anim : anim_list)
		delete anim;
	anim_list.clear();
	//停止音乐chunk
	Mix_HaltChannel(-1);
}

void Doodle::on_update(float delta)
{
	ImGui::BeginChild("dance", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border);
	ImGui::Image(texture_target, ImGui::GetContentRegionAvail());
	ImGui::EndChild();

	for (auto anim : anim_list)
	{
		//if (anim->get_cur_idx() == 0)
		//{
		//	//从头播放音乐
		//	Mix_HaltChannel(-1);
		//	Mix_PlayChannel(-1, ResourcesManager::instance()->find_audio(u8"doodle_dance"), -1);

		//}
		anim->on_update(delta);
	}
}

void Doodle::on_render(SDL_Renderer* renderer)
{
	SDL_SetRenderTarget(renderer, texture_target);
	SDL_SetRenderDrawColor(renderer, 15, 15, 15, 255);
	SDL_RenderClear(renderer);

	for (auto anim : anim_list)
		anim->on_render(renderer);

	SDL_SetRenderTarget(renderer, nullptr);
}