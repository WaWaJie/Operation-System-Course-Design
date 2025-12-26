#pragma once

#include"almighty_head.h"


class ProcessCommunication :public Example
{
public:

	void on_enter() override;
	void on_update(float delta)override;
	void on_input(const SDL_Event* event)override;
	void on_exit() override;

private:

	void on_update_imgui_region1(float delta);
	void on_update_imgui_region2(float delta);
	void on_update_imgui_region3(float delta);

	void on_update_process(float delta);
	void render_text_window();

private:
	void send(const std::string& id, const std::string& from, const std::string& to, const std::string& message);
	void receive(const std::string& id, const std::string& from, const std::string& to);
	void clear_message_box();


private:
	float sum_time = 0;			//程序自运行开始的总时间
	int process_idx = 1;		//进程索引
	bool can_execute = false;	//是否可以执行

	int algo_choose_id = 0;		//算法选择id

	float process_time = 1.0f;	//进程处理时间

	PCB* current_pcb = nullptr;
	std::vector<std::shared_ptr<PCB>>pcb_queue_normal;

	std::string str_tip = "";
	std::vector<TextString>str_tip_list;

	std::ifstream file_message_box_read;
	std::ofstream file_message_box_write;

	char msg_from[32] = u8"进程1";
	char msg_to[32] = u8"进程1";
	char msg_receive_from[32] = u8"进程1";
	char msg_id[64] = u8"msg_001";
	char msg_content[1024] = u8"From WaWaJie...";

	bool show_text_tip_window = false;
};