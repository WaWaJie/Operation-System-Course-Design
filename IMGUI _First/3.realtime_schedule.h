#pragma once

#include"almighty_head.h"


//实现LLF和EDF
class RealtimeSchedule :public Example
{
public:

	void on_enter() override;
	void on_update(float delta)override;
	void on_exit() override;

private:

	void on_update_imgui_region1(float delta);
	void on_update_imgui_region2(float delta);
	void on_update_imgui_region3(float delta);

	void on_update_algo(float delta);
	void on_update_ready(float delta);

private:
	struct RealtimeInfo
	{
		float ready_time = 0.0f;
		float deadline_start = 0.0f;
		float deadline_end = 3.0f;
		float process_time = 1.0f;
		//资源要求
	};


private:
	float sum_time = 0;

	int algo_choose_id = 0;
	int type_choose_id = 0;

	int process_idx = 1;

	PCB* current_pcb = nullptr;
	std::vector<PCB*>pcb_queue_normal;
	std::vector<PCB*>pcb_queue_sort;

	bool can_execute = false;

	std::string str_tip = "";

	RealtimeInfo realtime_info;
	std::vector<TextString>str_tip_list;
};