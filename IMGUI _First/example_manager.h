#pragma once

#include"example.h"
#include"singleton.h"
#include"resources_manager.h"

#include<string>
#include<vector>
#include<unordered_map>

#include<SDL.h>
#include<SDL_image.h>
#include<SDL2_gfxPrimitives.h>

class ExampleManager:public Singleton<ExampleManager>
{
	friend class Singleton<ExampleManager>;
public:
	void on_init(SDL_Renderer* renderer);

	void on_input(const SDL_Event* event);
	void on_render();
    void on_update(float delta);

private:
	struct MenuItem
	{
		std::string id;
		SDL_Texture* icon = nullptr;
		std::string title;

		MenuItem(const std::string& id, SDL_Texture* icon, const std::string& title)
			: id(id), icon(icon), title(title) {}
	};
	struct Subject
	{
		std::string title;
		std::vector<MenuItem> item_list;
	};

private:
	SDL_Renderer* renderer = nullptr;

	std::string current_example_id;
	Example*current_example = nullptr;

	SDL_Texture*texture_target = nullptr;
	std::unordered_map<std::string, Example*>example_pool;
	Subject subject_creational, subject_structural, subject_behavioral;

	SDL_Texture*texture_blank_content = nullptr;	


private:
	ExampleManager();
    ~ExampleManager();

	void on_update_blank_content();
	void switch_to(const std::string& id);
	void on_update_subject(const Subject& subject);
	void add_example(Subject& subject, const MenuItem& item, Example* example);

};