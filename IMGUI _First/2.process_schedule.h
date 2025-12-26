#pragma once

#include"almighty_head.h"


class ProcessSchedule :public Example
{
public:

	void on_enter() override;
	void on_update(float delta)override;	
	void on_exit() override;

	void on_update_imgui_region1(float delta);
	void on_update_imgui_region2(float delta);
	void on_update_imgui_region3(float delta);

	void on_rotate_update(float delta);	
	void on_priority_update(float delta);

private:
	float sum_time = 0;

	int algo_choose_id = 0;
	int type_choose_id = 0;

	int process_idx = 1;
	float time_rotate = 0.5f;
	float time_execute = 0.5f;		
	int priority = 1;
	int fair_ratio = 1;

	PCB*current_pcb = nullptr;
	std::vector<PCB*>pcb_list[11];
	std::vector<PCB*>pcb_queue_normal;

	int rotate_idx = 0;

	bool can_execute = false;
	
	std::string str_tip = "";
	std::vector<TextString>str_tip_list;

}; 