#include"2.process_schedule.h"

void ProcessSchedule::on_enter()
{
	sum_time = 0;

	algo_choose_id = 0;
	type_choose_id = 0;

	process_idx = 1;
	time_rotate = 0.5f;
	time_execute = 0.5f;
	priority = 1;
	fair_ratio = 1;

	current_pcb = nullptr;
	str_tip_list.clear();
}

void ProcessSchedule::on_exit()
{
	//释放内存，清空队列
	for (int i = 1; i <= 10; i++)
	{
		for (auto& pcb : pcb_list[i])
		{
			delete pcb;
		}
		pcb_list[i].clear();
	}
	pcb_queue_normal.clear();
	str_tip_list.clear();
}

void ProcessSchedule::on_update(float delta)
{
	//Y:120
	on_update_imgui_region1(delta);
	//Y:360
	on_update_imgui_region2(delta);

	on_update_imgui_region3(delta);

	if (!can_execute)return;
	sum_time += delta;
	switch (algo_choose_id)
	{
	case 0:
	case 3:
		on_rotate_update(delta);
		break;
	case 1:
	case 2:
		on_priority_update(delta);
		break;
	default:
		break;
	}
}

void ProcessSchedule::on_update_imgui_region1(float delta)
{
	ImGui::BeginChild("process_schedule_region1", { ImGui::GetContentRegionAvail().x,120 });

	ImGui::BeginGroup();
	ImGui::RadioButton(u8"非抢占式", &type_choose_id, 0); ImGui::SameLine();
	ImGui::RadioButton(u8"抢占式", &type_choose_id, 1);
	ImGui::EndGroup();

	ImGui::BeginGroup();
	ImGui::RadioButton(u8"轮转调度算法", &algo_choose_id, 0); ImGui::SameLine();
	ImGui::RadioButton(u8"优先级调度算法(FCFS)", &algo_choose_id, 1); ImGui::SameLine();
	/*ImGui::RadioButton(u8"多级反馈队列调度算法", &algo_choose_id, 2); ImGui::SameLine();*/
	ImGui::RadioButton(u8"基于公平原则调度算法", &algo_choose_id, 3);
	ImGui::EndGroup();

	ImGui::Text(u8"轮转时间："); ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::SliderFloat("##process_schedule_time_rotate", &time_rotate, 0.1, 5, "%.1f");
	ImGui::Text(u8"处理时间:"); ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::SliderFloat("##process_schedule_time_execute", &time_execute, 0.1, 5, "%.1f"); ImGui::SameLine();
	ImGui::Text(u8"优先级:"); ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::SliderInt("##process_schedule_priority", &priority, 0, 10); ImGui::SameLine();
	ImGui::Text(u8"公平占比:"); ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::SliderInt("##process_schedule_fair_ratio", &fair_ratio, 1, 10, "%.1i"); ImGui::SameLine();
	ImGui::Button(u8"添加进程"); ImGui::SameLine();
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
			pcb->priority = priority;
			pcb->fair_ratio = fair_ratio;
			pcb->time_execute = time_execute;
			pcb->timer_time_tick.set_wait_time(time_rotate * fair_ratio);
			pcb->timer_execute.set_wait_time(time_execute);
			pcb->update_state(ProcessState::ActiveReady);

			pcb_list[priority].push_back(pcb);
			pcb_queue_normal.push_back(pcb);

			std::string tip = std::to_string(sum_time) + u8"---进程创建: " + pcb->pname + ": " + pcb->state_name
				+ u8"--处理时间: " + std::to_string(pcb->time_execute)
				+ u8"--优先级:" + std::to_string(pcb->priority)
				+ u8"--公平比:" + std::to_string(pcb->fair_ratio);
			str_tip_list.push_back(TextString(tip, TextType::Info));
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
		else
		{
			switch (algo_choose_id)
			{
			case 0:
			case 3:
				for (int i = 0; i < pcb_queue_normal.size(); i++)
				{
					if (pcb_queue_normal[i]->current_state == ProcessState::ActiveReady)
					{
						rotate_idx = i;
						current_pcb = pcb_queue_normal[rotate_idx];
						break;
					}
				}
				if (!current_pcb)
				{
					std::string tip = std::to_string(sum_time) + u8"---Error: 无可执行的进程";
					str_tip_list.push_back(TextString(tip, TextType::Error));
					break;
				}
				current_pcb->update_state(ProcessState::Execute);
				break;
			case 1:
			case 2:
				for (int i = 1; i <= 10; i++)
				{
					if (current_pcb)break;
					for (int j = 0; j < pcb_list[i].size(); j++)
					{
						if (pcb_list[i][j]->current_state == ProcessState::ActiveReady)
						{
							current_pcb = pcb_list[i][j];
							current_pcb->update_state(ProcessState::Execute);
							break;
						}
					}
				}
				break;

			default:
				break;
			}
		}
	}

	ImGui::EndChild();
	ImGui::Separator();
}

void ProcessSchedule::on_update_imgui_region2(float delta)
{
	ImGui::BeginChild("process_schedule_region2", { ImGui::GetContentRegionAvail().x,400 });

	static const float height_delta = 44;
	for (int i = 0; i < pcb_queue_normal.size(); i++)
	{
		std::string label = pcb_queue_normal[i]->pname + ": " + pcb_queue_normal[i]->state_name
			+ u8"--处理时间:" + std::to_string(pcb_queue_normal[i]->time_execute)
			+ u8"--优先级:" + std::to_string(pcb_queue_normal[i]->priority)
			+ u8"--公平比:" + std::to_string(pcb_queue_normal[i]->fair_ratio);
		ImGui::Button(label.c_str(), { 400,40 });

		pcb_queue_normal[i]->draw_state({ 650,162 + height_delta * i }, 10);
	}


	ImGui::EndChild();
	ImGui::Separator();
}

void ProcessSchedule::on_update_imgui_region3(float delta)
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
		ImGui::SetScrollY(ImGui::GetScrollMaxY() + 20);
		last_tip_count = str_tip_list.size();
	}

	ImGui::EndChildFrame();
}


void ProcessSchedule::on_rotate_update(float delta)
{
	if (!can_execute || !current_pcb)return;
	//当前进程执行
	current_pcb->timer_time_tick.on_update(delta);
	current_pcb->timer_execute.on_update(delta);
	//进程执行结束或者时间片完（轮转周期结束），需要寻找就绪队列中的下一个进程
	if (current_pcb->current_state != ProcessState::Execute)
	{
		std::string tip = std::to_string(sum_time) + u8"---执行结束---" + current_pcb->pname + u8"  到达状态: " + current_pcb->state_name;
		str_tip_list.push_back(TextString(tip, TextType::Info));

		int count = 0;
		while (count < pcb_queue_normal.size())
		{
			rotate_idx = (rotate_idx + 1) % pcb_queue_normal.size();
			if (pcb_queue_normal[rotate_idx]->current_state == ProcessState::ActiveReady)
			{
				pcb_queue_normal[rotate_idx]->update_state(ProcessState::Execute);
				current_pcb = pcb_queue_normal[rotate_idx];

				std::string startTip = std::to_string(sum_time) + u8"---开始执行---" + pcb_queue_normal[rotate_idx]->pname;
				str_tip_list.push_back(TextString(startTip, TextType::Info));

				break;
			}
			count++;
		}
		if (count == pcb_queue_normal.size())
		{
			current_pcb->update_state(ProcessState::Finish);
			current_pcb = nullptr;
		}
	}
}

void ProcessSchedule::on_priority_update(float delta)
{
	if (!can_execute || !current_pcb)return;
	if (current_pcb->current_state == ProcessState::Execute)
		current_pcb->timer_execute.on_update(delta);

	if (current_pcb->current_state == ProcessState::Execute)
		return;

	//按照队列优先级调度任务，同一队列下FCFS
	for (int i = 1; i <= 10; i++)
	{
		for (int j = 0; j < pcb_list[i].size(); j++)
		{
			if (pcb_list[i][j]->current_state == ProcessState::ActiveReady)
			{
				std::string endTip = std::to_string(sum_time) + u8"---执行结束---" + current_pcb->pname + u8"  到达状态: " + current_pcb->state_name;
				str_tip_list.push_back(TextString(endTip, TextType::Info));

				current_pcb = pcb_list[i][j];
				current_pcb->current_state = ProcessState::Execute;

				std::string startTip = std::to_string(sum_time) + u8"---开始执行---" + pcb_list[i][j]->pname;
				str_tip_list.push_back(TextString(startTip, TextType::Info));

				return;
			}
		}
	}
	if (current_pcb->current_state == ProcessState::Finish)
		current_pcb = nullptr;
}