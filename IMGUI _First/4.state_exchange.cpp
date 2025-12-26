#include"4.state_exchange.h"
#include"config_manager.h"
#include<imgui.h>

StateExchange::StateExchange()
{
	current_pcb = new PCB();
	current_pcb->pname = u8"进程1";
	current_pcb->update_state(ProcessState::StaticBlock);
	pcb_list.push_back(current_pcb);
	str_tip_list.clear();
}

void StateExchange::on_update(float delta)
{
	//当前进程是否执行
	for (int i = 0; i < pcb_list.size(); i++)
	{
		ProcessState state_before = pcb_list[i]->current_state;
		if (pcb_list[i]->current_state != ProcessState::Execute)continue;
		pcb_list[i]->timer_time_tick.on_update(delta);
		pcb_list[i]->timer_execute.on_update(delta);
		if (pcb_list[i]->current_state == ProcessState::Finish)
		{
			pcb_list[i]->update_state(ProcessState::Finish);
			std::string tip = u8"进程结束------" + pcb_list[i]->pname + u8"：" + pcb_list[i]->state_name;
			str_tip_list.push_back(TextString(tip, TextType::Info));
		}
		//原本是正在执行的，执行之后变成了动态阻塞，说明时间片完
		if (pcb_list[i]->current_state == ProcessState::ActiveReady && state_before == ProcessState::Execute)
		{
			std::string tip = u8"时间片完------" + pcb_list[i]->pname + u8"：" + pcb_list[i]->state_name;
			str_tip_list.push_back(TextString(tip, TextType::Info));
		}
	}

	on_update_imgui_region1(delta);
	on_update_imgui_region2(delta);
	on_update_imgui_region3(delta);
}

void StateExchange::on_update_imgui_region1(float delta)
{
	{
		ImGui::BeginChild("state_exchange_region_1", { ImGui::GetContentRegionAvail().x, 100 });
		str_current_state_name = current_pcb->pname + ": " + current_pcb->state_name;
		ImGui::Text(str_current_state_name.c_str());
		//绘制一个实心圆
		current_pcb->draw_state(ImVec2(330, 26), 5);

		bool is_click_true = false;
		ImGui::Button(u8"挂起", { 80, 35 }); ImGui::SameLine();
		if (ImGui::IsItemClicked())
		{
			if (current_pcb->current_state == ProcessState::ActiveBlock)
			{
				current_pcb->update_state(ProcessState::StaticBlock);
				is_click_true = true;
			}
			else if (current_pcb->current_state == ProcessState::ActiveReady)
			{
				current_pcb->update_state(ProcessState::StaticReady);
				is_click_true = true;
			}
			else if (current_pcb->current_state == ProcessState::Execute)
			{
				current_pcb->update_state(ProcessState::StaticReady);
				is_click_true = true;
			}
		}
		ImGui::Button(u8"激活", { 80,35 }); ImGui::SameLine();
		if (ImGui::IsItemClicked())
		{
			if (current_pcb->current_state == ProcessState::StaticBlock)
			{
				current_pcb->update_state(ProcessState::ActiveBlock);
				is_click_true = true;
			}
			else if (current_pcb->current_state == ProcessState::StaticReady)
			{
				current_pcb->update_state(ProcessState::ActiveReady);
				is_click_true = true;
			}

		}
		ImGui::Button(u8"请求I/O", { 80,35 }); ImGui::SameLine();
		if (ImGui::IsItemClicked())
		{
			if (current_pcb->current_state == ProcessState::Execute)
			{
				current_pcb->update_state(ProcessState::ActiveBlock);
				is_click_true = true;
				current_pcb->timer_time_tick.restart();
			}
		}
		ImGui::Button(u8"调度", { 80,35 }); ImGui::SameLine();
		if (ImGui::IsItemClicked())
		{
			if (current_pcb->current_state == ProcessState::ActiveReady)
			{
				current_pcb->update_state(ProcessState::Execute);
				is_click_true = true;
			}
		}
		ImGui::Button(u8"释放", { 80,35 });
		if (ImGui::IsItemClicked())
		{
			if (current_pcb->current_state == ProcessState::StaticBlock)
			{
				current_pcb->update_state(ProcessState::StaticReady);
				is_click_true = true;
			}
			else if (current_pcb->current_state == ProcessState::ActiveBlock)
			{
				current_pcb->update_state(ProcessState::ActiveReady);
				is_click_true = true;
			}
		}
		if (is_click_true)
		{
			std::string tip = current_pcb->pname + u8"：" + current_pcb->state_name;
			str_tip_list.push_back(TextString(tip, TextType::Info));
		}


		ImGui::BeginGroup();
		ImGui::RadioButton(u8"静止就绪", &choose_ready, 0); ImGui::SameLine();
		ImGui::RadioButton(u8"活动就绪", &choose_ready, 1);
		ImGui::EndGroup(); ImGui::SameLine();
		ImGui::Button(u8"创建并许可", { 180,35 });
		if (ImGui::IsItemClicked())
		{
			if (pcb_list.size() >= 8)
			{
				std::string tip = u8"Error：进程数已满，创建失败";
				str_tip_list.push_back(TextString(tip, TextType::Error));
			}
			else
			{
				PCB* new_pcb = new PCB();
				if (choose_ready == 0)
				{
					new_pcb->update_state(ProcessState::StaticReady);
					new_pcb->pname = u8"进程" + std::to_string(pcb_list.size() + 1);
				}
				else
				{
					new_pcb->update_state(ProcessState::ActiveReady);
					new_pcb->pname = u8"进程" + std::to_string(pcb_list.size() + 1);
				}
				pcb_list.push_back(new_pcb);
				std::string tip = u8"创建新进程：" + new_pcb->pname + u8"：" + new_pcb->state_name;
				str_tip_list.push_back(TextString(tip, TextType::Info));
			}
		}

		ImGui::EndChild();
	}
}

void StateExchange::on_update_imgui_region2(float delta)
{
	ImGui::SeparatorText(u8"进程状态");
	{
		static const int height_delta = 44;
		ImGui::BeginChild("state_exchange_region_2", { ImGui::GetContentRegionAvail().x,360 });
		for (int i = 0; i < pcb_list.size(); i++)
		{
			str_state_describe = pcb_list[i]->pname + ": " + pcb_list[i]->state_name;

			ImGui::Button(str_state_describe.c_str(), { 200,40 });
			if (ImGui::IsItemClicked())
			{
				current_pcb = pcb_list[i];
			}

			pcb_list[i]->draw_state(ImVec2(450, 168 + height_delta * i), 10);
		}
		ImGui::EndChild();
	}
}

void StateExchange::on_update_imgui_region3(float delta)
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