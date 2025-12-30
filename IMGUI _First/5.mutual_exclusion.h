#pragma once

#include"almighty_head.h"


class MutexExclusion :public Example
{
public:

	void on_enter() override;
	void on_update(float delta)override;
	void on_exit() override;

private:

	void on_update_imgui_region1(float delta);
	void on_update_imgui_region2(float delta);
	void on_update_imgui_region3(float delta);

	void on_update_process(float delta);

private:
	

private:
	float sum_time = 0;
	int process_idx = 1;
	bool can_execute = false;

	int algo_choose_id = 0;
	
	float process_time = 1.0f;
	int resource_occupy = 0;

	int resource_to_add = 0;

	char resource_name[32] = "Resource_A";
	char resource_occupied_name[32] = "Resource_A";

	PCB* current_pcb = nullptr;
	std::vector<PCB*>pcb_queue_normal;		

	std::string str_tip = "";
	std::vector<TextString>str_tip_list;


};