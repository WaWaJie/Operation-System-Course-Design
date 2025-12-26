#include"3.realtime_schedule.h"

#include<sstream>
#include<iomanip>
#include<algorithm>

void RealtimeSchedule::on_enter()
{
	sum_time = 0;
	algo_choose_id = 0;
	type_choose_id = 0;
	process_idx = 1;
	current_pcb = nullptr;
	can_execute = false;
	str_tip_list.clear();
	realtime_info.ready_time = 0.0f;
	realtime_info.deadline_start = 0.0f;
	realtime_info.deadline_end = 3.0f;
	realtime_info.process_time = 1.0f;
}

void RealtimeSchedule::on_exit()
{
	for (auto& pcb : pcb_queue_normal)
		delete pcb;
	pcb_queue_normal.clear();
	str_tip_list.clear();
}


void RealtimeSchedule::on_update(float delta)
{
	on_update_imgui_region1(delta);
	on_update_imgui_region2(delta);
	on_update_imgui_region3(delta);

	if (!can_execute)return;
	sum_time += delta;

	on_update_ready(delta);
	on_update_algo(delta);

}

void RealtimeSchedule::on_update_imgui_region1(float delta)
{
	ImGui::BeginChild("realtime_schedule_region1", { ImGui::GetContentRegionAvail().x,120 });

	ImGui::BeginGroup();
	ImGui::RadioButton(u8"非抢占式", &type_choose_id, 0); ImGui::SameLine();
	ImGui::RadioButton(u8"抢占式", &type_choose_id, 1);
	ImGui::EndGroup();

	ImGui::BeginGroup();
	ImGui::RadioButton(u8"最早截止时间EDF", &algo_choose_id, 0); ImGui::SameLine();
	ImGui::RadioButton(u8"最低松弛度LLF", &algo_choose_id, 1);
	ImGui::EndGroup();
	
	ImGui::Text(u8"就绪时间"); ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::SliderFloat(u8"##real_time_ready_time", &realtime_info.ready_time, 0, 10, "%.1f"); ImGui::SameLine();
	/*ImGui::SetNextItemWidth(90);
	ImGui::SliderFloat(u8"开始截止时间", &realtime_info.deadline_start, 0, 10, "%.1f"); ImGui::SameLine();*/
	ImGui::Text(u8"截止时间"); ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::SliderFloat(u8"##real_time_deadline", &realtime_info.deadline_end, 0, 10, "%.1f"); ImGui::SameLine();
	ImGui::Text(u8"处理时间"); ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::SliderFloat(u8"##real_time_process_time", &realtime_info.process_time, 0.1f, 5, "%.1f");

	ImGui::Button(u8"创建进程"); ImGui::SameLine();

	if (ImGui::IsItemClicked())
	{
		if (pcb_queue_normal.size() >= 8)
		{
			std::string tip = std::to_string(sum_time) + u8"---进程创建失败: 进程数已达到最大值";
			str_tip_list.push_back(TextString(tip, TextType::Error));
		}
		else
		{
			PCB* pcb = new PCB();
			pcb->pname = u8"进程" + std::to_string(process_idx++);
			pcb->update_state(ProcessState::ActiveBlock);
			pcb->ready_time = realtime_info.ready_time;
			pcb->deadline_start = std::max(realtime_info.deadline_start, pcb->ready_time);
			pcb->deadline_end = std::max(realtime_info.deadline_end, pcb->deadline_start);
			pcb->time_execute = realtime_info.process_time;
			pcb->timer_execute.set_wait_time(pcb->time_execute);


			std::stringstream ss1, ss2, ss3, ss4;
			ss1 << std::fixed << std::setprecision(1) << pcb->ready_time;
			ss2 << std::fixed << std::setprecision(1) << pcb->deadline_start;
			ss3 << std::fixed << std::setprecision(1) << pcb->deadline_end;
			ss4 << std::fixed << std::setprecision(1) << pcb->time_execute;

			std::string tip = std::to_string(sum_time) + u8"---进程创建成功: " + pcb->pname + u8"---就绪时间:" + ss1.str()
				/*+ u8"--开始截止时间:" + ss2.str()*/
				+ u8"--截止时间:" + ss3.str()
				+ u8"--处理时间:" + ss4.str();
			str_tip_list.push_back(TextString(tip, TextType::Info));

			pcb_queue_normal.push_back(pcb);
			pcb_queue_sort.push_back(pcb);
		}
	}
	ImGui::Button(u8"开始执行"); ImGui::SameLine();
	if (ImGui::IsItemClicked())
	{
		can_execute = true;
		if (pcb_queue_normal.empty())
		{
			std::string tip = std::to_string(sum_time) + u8"---Error: 无可执行的进程";
			str_tip_list.push_back(TextString(tip, TextType::Error));
		}
	}

	ImGui::EndChild();
	ImGui::Separator();
}
void RealtimeSchedule::on_update_imgui_region2(float delta)
{
	ImGui::BeginChild("process_schedule_region2", { ImGui::GetContentRegionAvail().x,400 });

	static const float height_delta = 44;
	for (int i = 0; i < pcb_queue_normal.size(); i++)
	{
		auto& pcb = pcb_queue_normal[i];

		std::stringstream ss1, ss2, ss3, ss4;
		ss1 << std::fixed << std::setprecision(1) << pcb->ready_time;
		ss2 << std::fixed << std::setprecision(1) << pcb->deadline_start;
		ss3 << std::fixed << std::setprecision(1) << pcb->deadline_end;
		ss4 << std::fixed << std::setprecision(1) << pcb->time_execute;

		std::string label = pcb_queue_normal[i]->pname + ": " + pcb_queue_normal[i]->state_name
			+ u8"---就绪时间:" + ss1.str()
			/*+ u8"--开始截止时间:" + ss2.str()*/
			+ u8"--结束截止时间:" + ss3.str()
			+ u8"--处理时间:" + ss4.str();

		ImGui::Button(label.c_str(), { 600,40 });

		pcb_queue_normal[i]->draw_state({ 850,162 + height_delta * i }, 10);
	}


	ImGui::EndChild();
	ImGui::Separator();
}
void RealtimeSchedule::on_update_imgui_region3(float delta)
{
	ImGui::BeginChildFrame(1, { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y });
	ImGui::Button(u8"清空日志", { ImGui::GetContentRegionAvail().x,30 });
	if (ImGui::IsItemClicked())
	{
		str_tip_list.clear();
	}

	// 显示str_tip_list中的所有提示
	for (const auto& textString : str_tip_list)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, static_cast<ImVec4>(ConfigManager::instance()->get_text_color(textString.type)));
		ImGui::TextUnformatted(textString.text_info.c_str());
		ImGui::PopStyleColor();
	}


	static size_t last_tip_count = 0;
	if (last_tip_count != str_tip_list.size())
	{
		ImGui::SetScrollY(ImGui::GetScrollMaxY() + 2000);
		last_tip_count = str_tip_list.size();
	}

	ImGui::EndChildFrame();
}

void RealtimeSchedule::on_update_ready(float delta)
{
	for (auto& pcb : pcb_queue_normal)
	{
		if (pcb->ready_time <= sum_time && pcb->current_state == ProcessState::ActiveBlock)
		{
			std::string tip = std::to_string(sum_time) + u8"---状态转换---" + pcb->pname + ": " + pcb->state_name + "--->" + u8"活动就绪";
			str_tip_list.push_back(TextString(tip, TextType::Info));
			pcb->update_state(ProcessState::ActiveReady);
		}
	}
}

void RealtimeSchedule::on_update_algo(float delta)
{
	if (current_pcb && current_pcb->current_state == ProcessState::Execute)
	{
		current_pcb->timer_execute.on_update(delta);
	}

	switch (algo_choose_id) {
	case 0:
		sort(pcb_queue_sort.begin(), pcb_queue_sort.end(),
			[](PCB* p1, PCB* p2) { return p1->deadline_end < p2->deadline_end; });
		break;
	case 1:
		sort(pcb_queue_sort.begin(), pcb_queue_sort.end(),
			[&](PCB* p1, PCB* p2) {
				float laxity1 = p1->deadline_end - sum_time - p1->timer_execute.get_remain_time();
				float laxity2 = p2->deadline_end - sum_time - p2->timer_execute.get_remain_time();
				return laxity1 < laxity2;
			});
		break;
	default:
		break;
	}

	for (int i = 0; i < pcb_queue_sort.size(); i++)
	{
		//非抢占式：如果当前进程在执行中，就不更换进程
		if (type_choose_id == 0 && current_pcb && current_pcb->current_state == ProcessState::Execute)
			break;
		if (current_pcb && current_pcb->current_state == ProcessState::Finish)
		{
			std::string tip = std::to_string(sum_time) + u8"---状态转换---" + current_pcb->pname + u8"---已经完成";
			str_tip_list.push_back(TextString(tip, TextType::Info));
			current_pcb = nullptr;
		}
		auto& pcb = pcb_queue_sort[i];
		if (pcb == current_pcb)break;//避免优先级高的进程被次级进程抢占
		if (pcb->current_state == ProcessState::ActiveReady)
		{
			if (current_pcb && current_pcb->current_state == ProcessState::Execute)
				current_pcb->update_state(ProcessState::ActiveReady);
			current_pcb = pcb;
			current_pcb->update_state(ProcessState::Execute);
			std::string tip = std::to_string(sum_time) + u8"---进程切换---" + pcb->pname + u8"---开始执行";
			str_tip_list.push_back(TextString(tip, TextType::Info));
			break;
		}
	}
}
