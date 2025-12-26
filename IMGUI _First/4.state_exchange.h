#pragma once
#include"almighty_head.h"



class StateExchange : public Example
{ 
public:
	StateExchange();

	//void on_enter()override;
	//void on_exit()override;
	void on_update(float delta)override;
	//void on_render(SDL_Renderer* renderer)override;

	void on_update_imgui_region1(float delta);
	void on_update_imgui_region2(float delta);
	void on_update_imgui_region3(float delta);

private:
	ProcessState current_state=ProcessState::ActiveBlock;
	
	PCB* current_pcb = nullptr;
	std::vector<PCB*>pcb_list;

	std::string str_current_state_name = "";
	std::string str_state_describe = "";
	std::string str_tip = "######################################################";
	std::vector<TextString>str_tip_list;
	
	int choose_ready = 0;
};

