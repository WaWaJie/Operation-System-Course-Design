#pragma once

#include<unordered_map>
#include<string>

#include<imgui.h>
#include<imgui_internal.h>
#include<imgui_impl_sdl.h>
#include<SDL.h>

#include"process_state.h"
#include"process_resource.h"
#include"singleton.h"
#include"text_string.h"

class ConfigManager : public Singleton<ConfigManager>
{
	friend class Singleton<ConfigManager>;
public:
	ConfigManager()
	{
		color_pool[ProcessState::ActiveBlock] = ImColor(255, 0, 0, 255);
		color_pool[ProcessState::ActiveReady] = ImColor(255, 255, 0, 255);
		color_pool[ProcessState::Execute] = ImColor(0, 255, 0, 255);
		color_pool[ProcessState::Finish] = ImColor(255, 0, 255, 255);
		color_pool[ProcessState::StaticBlock] = ImColor(95, 0, 0, 255);
		color_pool[ProcessState::StaticReady] = ImColor(95, 125, 0, 255);

		text_color_pool[TextType::Info] = ImColor(0, 255, 0, 255);
		text_color_pool[TextType::Warning] = ImColor(255, 255, 0, 255);
        text_color_pool[TextType::Error] = ImColor(255, 0, 0, 255);
		text_color_pool[TextType::Finish] = ImColor(255, 0, 255, 255);
		text_color_pool[TextType::KeyInfo] = ImColor(255, 215, 0, 255);

	}

	const std::unordered_map<ProcessState, ImColor>get_color_pool() const { return color_pool; }
	const std::unordered_map<std::string,int>get_resource_pool() const { return resource_pool; }

	void add_resource(std::string name, int value) { resource_pool[name] += value; }
	void sub_resource(std::string name, int value) { resource_pool[name] -= value; }
	int get_resource_number(std::string name) { return resource_pool[name]; }

	void add_resource_max(std::string name, int value) { resource_pool_max[name] += value; }
	void sub_resource_max(std::string name, int value) { resource_pool_max[name] -= value; }
	int get_resource_number_max(std::string name) { return resource_pool_max[name]; }

	void clear_resource() { resource_pool.clear(); resource_pool_max.clear(); }

	ImColor get_text_color(TextType type) { return text_color_pool[type]; }
	
private:
	std::unordered_map<ProcessState, ImColor> color_pool;
	std::unordered_map<std::string, int>resource_pool;
	std::unordered_map<std::string, int>resource_pool_max;

	std::unordered_map<TextType, ImColor>text_color_pool;
};