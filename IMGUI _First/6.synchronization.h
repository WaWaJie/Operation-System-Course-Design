#pragma once
#include"almighty_head.h"


class Synchronization :public Example
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
	void on_update_algo_record(float delta);
	void on_update_algo_and(float delta);
	
private:
	float sum_time = 0;			//程序自运行开始的总时间
	int process_idx = 1;		//进程索引
	bool can_execute = false;	//是否可以执行

	int algo_choose_id = 0;		//算法选择id

	float process_time = 1.0f;	//进程处理时间
	int resource_occupy = 0;	//占用资源量

	//需要添加的资源量
	int resource_to_add = 0;

	//设计其实重复了，不过为了方便就这样写了
	char resource_name[32] = "Resource_A";
	char resource_occupied_name[32] = "Resource_A";

	PCB* current_pcb = nullptr;
	std::vector<std::shared_ptr<PCB>>pcb_queue_normal;
	std::vector<std::shared_ptr<PCB>>pcb_queue_can_execute;//可执行队列
	int pcb_idx_execute = 0;//可执行队列中当前执行的进程索引
	
	std::string str_tip = "";
	std::vector<TextString>str_tip_list;


};