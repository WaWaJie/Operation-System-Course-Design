#include"cyy.h"
#include"resources_manager.h"
#include<random>
#include<iostream>
#include<SDL_mixer.h>

CYY::CYY(SDL_Renderer*renderer)
{
	this->renderer = renderer;
	texture_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 332, 296);
}
CYY::~CYY()
{
	SDL_DestroyTexture(texture_target);
}

void CYY::on_enter()
{
	//随机数
	srand((unsigned int)time(NULL));
	//播放音乐chunk
	Mix_PlayChannel(-1, ResourcesManager::instance()->find_audio(u8"cyy"), -1);
	
}
void CYY::on_exit()
{
	for(auto anim : anim_list)
        delete anim;
    anim_list.clear();
	//停止音乐chunk
    Mix_HaltChannel(-1);
}

void CYY::on_update(float delta)
{
	ImGui::BeginGroup();
	ImGui::RadioButton("长夜月舞1", &choose_id, 1); ImGui::SameLine();
	ImGui::RadioButton("长夜月舞2", &choose_id, 2);ImGui::SameLine();
    ImGui::RadioButton("长夜月舞3", &choose_id, 3);
	ImGui::RadioButton("长夜月舞4", &choose_id, 4); ImGui::SameLine();
	ImGui::RadioButton("长夜月舞5", &choose_id, 5); ImGui::SameLine();
	ImGui::RadioButton("长夜月舞6", &choose_id, 6);
	ImGui::EndGroup();		ImGui::SameLine();

	ImGui::Button("随机生成", { ImGui::GetContentRegionAvail().x , 50.0 });
	//按下按钮之后
    if (ImGui::IsItemClicked())
    {
		Animation* animation = nullptr;
		//生成[0,400]的随机数
        int xx = rand() % 200;
		int yy = rand() % 200;
		float x = xx, y = yy;
		switch (choose_id)
		{
		case 1:
			animation = new Animation(renderer, ResourcesManager::instance()->find_animation(u8"c1"), { x,y });
            animation->set_interval(0.1f);
            animation->set_ratio(1.0f);
			anim_list.push_back(animation);
			break;
		case 2:
            animation = new Animation(renderer, ResourcesManager::instance()->find_animation(u8"c2"), { x,y });
            animation->set_interval(0.1f);
            animation->set_ratio(1.0f);
            anim_list.push_back(animation);
            break;
		case 3:
            animation = new Animation(renderer, ResourcesManager::instance()->find_animation(u8"c3"), { x,y });
            animation->set_interval(0.1f);
            animation->set_ratio(1.0f);
            anim_list.push_back(animation);
            break;
		case 4:
			animation = new Animation(renderer, ResourcesManager::instance()->find_animation(u8"c4"), { x,y });
			animation->set_interval(0.1f);
			animation->set_ratio(1.0f);
			anim_list.push_back(animation);
			break;
		case 5:
			animation = new Animation(renderer, ResourcesManager::instance()->find_animation(u8"c5"), { x,y });
			animation->set_interval(0.1f);
			animation->set_ratio(1.0f);
			anim_list.push_back(animation);
			break;
		case 6:
			animation = new Animation(renderer, ResourcesManager::instance()->find_animation(u8"c6"), { x,y });
			animation->set_interval(0.1f);
			animation->set_ratio(1.0f);
			anim_list.push_back(animation);
			break;
		default:
			break;
		}
    }

	//生成一道分割线
    ImGui::Separator();

	ImGui::BeginChild("dance", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border);
	ImGui::Image(texture_target, ImGui::GetContentRegionAvail());
	ImGui::EndChild();

	for(auto anim : anim_list)
        anim->on_update(delta);
}

void CYY::on_render(SDL_Renderer* renderer)
{
	SDL_SetRenderTarget(renderer, texture_target);
	SDL_SetRenderDrawColor(renderer, 15, 15, 15, 255);
	SDL_RenderClear(renderer);
	
	for (auto anim : anim_list)
		anim->on_render(renderer);

	SDL_SetRenderTarget(renderer, nullptr);
}