#pragma once

#include"almighty_head.h"

class DeadLockPrevention :public Example
{
public:

	void on_enter() override;
	void on_update(float delta)override;
	void on_input(const SDL_Event* event);
	void on_exit() override;

private:

	void on_update_imgui_region1(float delta);
	void on_update_imgui_region2(float delta);
	void on_update_imgui_region3(float delta);

	void on_update_process(float delta);
	void render_window_text();
	void render_window_process_add(bool&flag);

private:


private:
	float sum_time = 0;			//程序自运行开始的总时间
	int process_idx = 1;		//进程索引
	bool can_execute = false;	//是否可以执行

	int algo_choose_id = 0;		//算法选择id

	float process_time = 2.0f;	//进程处理时间

	PCB* current_pcb = nullptr;		//当前选中的进程控制块
	std::vector<std::shared_ptr<PCB>>pcb_queue_normal;	//进程控制块队列

	//std::string str_tip = "";	//运行提示信息
	std::vector<TextString>str_tip_list;

	bool show_window_text = false;

	int num_of_resource_to_add = 1;
	int num_of_resource_process_request[4] = { 0 };
	int id_of_resource_process_request[4] = { 0 };

	const std::vector<std::string>resource_name_list = { "ResourceA","ResourceB","ResourceC","ResourceD" };
};