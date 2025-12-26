#include"8.deadlock.h"

void DeadLock::on_enter()
{
	str_tip = "";
	process_idx = 1;
	sum_time = 0;
	algo_choose_id = 0;
	current_pcb = nullptr;
	can_execute = false;
}

void DeadLock::on_exit()
{
	ConfigManager::instance()->clear_resource();
	pcb_queue_normal.clear();
}

void DeadLock::on_input(const SDL_Event* event)
{
	if (ImGui::GetIO().WantCaptureKeyboard)return;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_e)
	{
		show_window_text=!show_window_text;
	}
}


void DeadLock::on_update(float delta)
{
	on_update_imgui_region1(delta);
	on_update_imgui_region2(delta);
	on_update_imgui_region3(delta);

	render_window_text();
	if (!can_execute)return;
	sum_time += delta;
	on_update_process(delta);
}

void DeadLock::on_update_imgui_region1(float delta)
{
	ImGui::BeginChild("process_communication_region1", { ImGui::GetContentRegionAvail().x,120 });


	ImGui::EndChild();
	ImGui::Separator();
}

void DeadLock::on_update_imgui_region2(float delta)
{
	ImGui::BeginChild("process_communication_region2", { ImGui::GetContentRegionAvail().x,400 });

	static const float height_delta = 44;
	for (int i = 0; i < pcb_queue_normal.size(); i++)
	{
		auto& pcb = pcb_queue_normal[i];

		std::stringstream ss;
		ss << std::fixed << std::setprecision(1) << pcb->time_execute;

		std::string label = pcb_queue_normal[i]->pname + ": " + pcb_queue_normal[i]->state_name
			+ u8"--处理时间:" + ss.str();

		ImGui::Button(label.c_str(), { 600,40 });
		if (ImGui::IsItemClicked())
		{
			current_pcb = pcb_queue_normal[i].get();
		}

		pcb_queue_normal[i]->draw_state({ 850,162 + height_delta * i }, 10);
	}

	ImGui::EndChild();
	ImGui::Separator();
}

void DeadLock::on_update_imgui_region3(float delta)
{
	ImGui::BeginChildFrame(1, { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y });
	ImGui::Button(u8"清空日志", { ImGui::GetContentRegionAvail().x,30 });
	if (ImGui::IsItemClicked())
	{
		str_tip = "";
	}
	ImGui::TextUnformatted(str_tip.c_str());

	static std::string str_last_tip = str_tip;
	if (str_last_tip != str_tip)
	{
		ImGui::SetScrollY(ImGui::GetScrollMaxY() + 2000);
	}
	str_last_tip = str_tip;

	ImGui::EndChildFrame();
}

void DeadLock::on_update_process(float delta)
{
	switch (algo_choose_id)
	{
	case 0:
		break;
	case 1:
		break;
	default:
		break;
	}
}

void DeadLock::render_window_text()
{
	if(!show_window_text)return;
	ImGui::SetNextWindowSize({ 600, 300 });
	if (ImGui::Begin(u8"死锁处理系统说明", &show_window_text))
	{
		ImGui::BeginChild("DeadlockTipContent", ImVec2(0, 0), true);
		{
			ImGui::PushTextWrapPos(0.0f);

			ImGui::Image(ResourcesManager::instance()->find_texture("xielunyan"), { 20, 20 }); ImGui::SameLine();
			ImGui::Text(u8"产生死锁的必要条件:\n（1）互斥条件（2）请求和保持条件（3）不可抢占条件（4）循环等待条件");
			ImGui::Image(ResourcesManager::instance()->find_texture("xielunyan"), { 20, 20 }); ImGui::SameLine();
			ImGui::Text(u8"处理死锁的方法:\n（1）预防死锁（2）避免死锁（3）检测死锁（4）解除死锁");
			ImGui::Image(ResourcesManager::instance()->find_texture("xielunyan"), { 20, 20 }); ImGui::SameLine();
			ImGui::Text(u8"预防死锁:\n（1）破坏请求和保持条件（2）破坏不可抢占条件（3）破坏循环等待条件");
			ImGui::Image(ResourcesManager::instance()->find_texture("xielunyan"), { 20, 20 }); ImGui::SameLine();
			ImGui::Text(u8"破坏请求和保持条件:\n（1）第一种协议（一次性申请资源）（2）第二种协议（分批次申请）");
			ImGui::Image(ResourcesManager::instance()->find_texture("xielunyan"), { 20, 20 }); ImGui::SameLine();
			ImGui::Text(u8"破坏不可抢占条件:\n资源请求若无法得到满足，则释放所有已经持有的资源");
			ImGui::Image(ResourcesManager::instance()->find_texture("xielunyan"), { 20, 20 }); ImGui::SameLine();
			ImGui::Text(u8"破坏循环等待条件:\n将系统资源进行排序，并规定进程只能按照一定的顺序申请资源");

			ImGui::PopTextWrapPos();
		}
		ImGui::EndChild();
	}
	ImGui::End();


}