#include"5.mutual_exclusion.h"

void MutexExclusion::on_enter()
{
	str_tip_list.clear();
	process_idx = 1;
	sum_time = 0;
	algo_choose_id = 0;
	current_pcb = nullptr;
	can_execute = false;
}

void MutexExclusion::on_exit()
{
	ConfigManager::instance()->clear_resource();
	pcb_queue_normal.clear();
	str_tip_list.clear();
}

void MutexExclusion::on_update(float delta)
{
	on_update_imgui_region1(delta);
	on_update_imgui_region2(delta);
	on_update_imgui_region3(delta);

	if (!can_execute)return;
	sum_time += delta;
	on_update_process(delta);
}

void MutexExclusion::on_update_imgui_region1(float delta)
{
	ImGui::BeginChild("mutex_exclusion_region1", { ImGui::GetContentRegionAvail().x,120 });

	ImGui::Text(u8"资源名称: "); ImGui::SameLine();

	ImGui::SetNextItemWidth(150);
	static const char* items[] = { "Resource_A", "Resource_B", "Resource_C" ,"Resource_D" ,"Resource_E" ,"Resource_F" ,"Resource_G" ,"Resource_H" };
	static int item_current = 0;
	if (ImGui::Combo("##mutex_exclusion_resource_name_listbox", &item_current, items, IM_ARRAYSIZE(items), 4))
	{
		strcpy_s(resource_name, items[item_current]);
		strcpy_s(resource_occupied_name, items[item_current]);
	}ImGui::SameLine();

	ImGui::Text(u8"资源量: "); ImGui::SameLine();
	ImGui::SetNextItemWidth(150);
	ImGui::InputInt("##mutex_exclusion_resource_amount_input", &resource_to_add);
	ImGui::SameLine();
	ImGui::Button(u8"添加资源"); ImGui::SameLine();
	if (ImGui::IsItemClicked())
	{
		std::string rname = resource_name;
		ConfigManager::instance()->add_resource(rname, resource_to_add);
		ConfigManager::instance()->add_resource_max(rname, resource_to_add);
		std::string tip = std::to_string(sum_time) + u8"---系统资源 " + rname + u8" 增加了 " + std::to_string(resource_to_add) + u8" 个";
		str_tip_list.push_back(TextString(tip, TextType::Info));
	}
	ImGui::Button(u8"删除资源");
	if (ImGui::IsItemClicked())
	{
		std::string rname = resource_name;
		ConfigManager::instance()->sub_resource(rname, resource_to_add);
		ConfigManager::instance()->sub_resource_max(rname, resource_to_add);
		std::string tip = std::to_string(sum_time) + u8"---系统资源 " + rname + u8" 减少了 " + std::to_string(resource_to_add) + u8" 个";
		str_tip_list.push_back(TextString(tip, TextType::Info));
	}

	ImGui::Text(u8"占用资源名称:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(110);
	if (ImGui::Combo("##mutex_exclusion_resource_occupied_name_listbox", &item_current, items, IM_ARRAYSIZE(items), 4))
	{
		strcpy_s(resource_name, items[item_current]);
		strcpy_s(resource_occupied_name, items[item_current]);
	} ImGui::SameLine();
	ImGui::Text(u8"占用资源量:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	ImGui::SliderInt("##mutex_exclusion_resource_occupy", &resource_occupy, 0, 10, "%d"); ImGui::SameLine();
	ImGui::Text(u8"处理时间:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	ImGui::SliderFloat("##mutual_exclusion_process_time", &process_time, 0.1f, 5.0f, "%.1f"); ImGui::SameLine();

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
			auto pcb = std::make_shared<PCB>();
			pcb->time_execute = process_time;
			pcb->pname = u8"进程" + std::to_string(process_idx++);
			pcb->update_state(ProcessState::ActiveReady);

			pcb->resource_occupied_name = resource_occupied_name;
			pcb->resource_max[resource_occupied_name] = resource_occupy;
			pcb->resource_hold[resource_occupied_name] = 0;

			pcb->timer_time_tick.set_wait_time(0.1f);
			pcb->timer_time_tick.set_one_shot(false);
			pcb->timer_execute.set_wait_time(process_time);
			pcb->timer_execute.set_one_shot(true);
			auto sp = pcb;
			pcb->timer_execute.set_on_timeout([&, sp]()
				{
					ConfigManager::instance()->add_resource(sp->resource_occupied_name, sp->resource_hold[sp->resource_occupied_name]);
					sp->update_state(ProcessState::Finish);
					std::string tip = std::to_string(sum_time) + u8"---进程结束:  " + sp->pname;
					str_tip_list.push_back(TextString(tip, TextType::Info));
				});

			pcb_queue_normal.push_back(pcb.get());
			std::string tip = std::to_string(sum_time) + u8"---进程创建成功: " + pcb->pname
				+ u8"---占用资源名称:" + pcb->resource_occupied_name
				+ u8"--占用资源量:" + std::to_string(pcb->resource_max[pcb->resource_occupied_name])
				+ u8"--处理时间:" + std::to_string(process_time);
			str_tip_list.push_back(TextString(tip, TextType::Info));
		}
	}
	ImGui::Button(u8"开始执行");
	if (ImGui::IsItemClicked())
	{
		can_execute = true;
		if (pcb_queue_normal.empty())
		{
			std::string tip = std::to_string(sum_time) + u8"---Error: 无可执行的进程";
			str_tip_list.push_back(TextString(tip, TextType::Error));
		}
	}

	const auto& resource_pool = ConfigManager::instance()->get_resource_pool();
	float bar_height = 20.0f;     // 进度条高度

	for (auto& resource : resource_pool)
	{
		const auto resource_number_max = ConfigManager::instance()->get_resource_number_max(resource.first);
		ImGui::Image(ResourcesManager::instance()->find_texture("Resource"), { 20,20 });
		ImGui::SameLine();

		ImGui::Text(u8"资源名称: %s", resource.first.c_str());
		ImGui::SameLine();

		ImGui::Text(u8"  资源量: %d/%d", resource.second, (int)resource_number_max);
		ImGui::SameLine();

		float avail_width = 200;
		ImGui::ProgressBar((float)resource.second / std::max(resource_number_max, 1), ImVec2(avail_width, bar_height));

	}

	ImGui::EndChild();
	ImGui::Separator();
}

void MutexExclusion::on_update_imgui_region2(float delta)
{
	ImGui::BeginChild("process_schedule_region2", { ImGui::GetContentRegionAvail().x,400 });

	static const float height_delta = 44;
	for (int i = 0; i < pcb_queue_normal.size(); i++)
	{
		auto& pcb = pcb_queue_normal[i];

		std::stringstream ss;
		ss << std::fixed << std::setprecision(1) << pcb->time_execute;

		std::string label = pcb_queue_normal[i]->pname + ": " + pcb_queue_normal[i]->state_name
			+ u8"---占用资源名称:" + pcb->resource_occupied_name
			+ u8"--占用资源量:" + std::to_string(pcb->resource_max[pcb->resource_occupied_name])
			+ u8"--处理时间:" + ss.str();

		ImGui::Button(label.c_str(), { 600,40 });

		pcb_queue_normal[i]->draw_state({ 850,162 + height_delta * i }, 10);
	}

	ImGui::EndChild();
	ImGui::Separator();
}

void MutexExclusion::on_update_imgui_region3(float delta)
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

void MutexExclusion::on_update_process(float delta)
{
	for (int i = 0; i < pcb_queue_normal.size(); i++)
	{
		auto& pcb = pcb_queue_normal[i];
		if (pcb->current_state == ProcessState::Finish)continue;
		const auto& config = ConfigManager::instance();
		if (config->get_resource_number(pcb->resource_occupied_name)
			>= pcb->resource_max[pcb->resource_occupied_name]
			- pcb->resource_hold[pcb->resource_occupied_name]
			|| pcb->resource_max[pcb->resource_occupied_name] == 0)
		{
			if (pcb->current_state == ProcessState::ActiveReady)
			{
				std::string tip = std::to_string(sum_time) + u8"---进程执行:  " + pcb->pname + u8"---占用资源:  "
					+ pcb->resource_occupied_name + u8"---数量:  "
					+ std::to_string(pcb->resource_max[pcb->resource_occupied_name]
						- pcb->resource_hold[pcb->resource_occupied_name]);
				str_tip_list.push_back(TextString(tip, TextType::Info));
			}
			//整体资源减少
			config->sub_resource(pcb->resource_occupied_name,
				pcb->resource_max[pcb->resource_occupied_name]
				- pcb->resource_hold[pcb->resource_occupied_name]);
			//进程持有资源增加
			pcb->resource_hold[pcb->resource_occupied_name] =
				pcb->resource_max[pcb->resource_occupied_name];
			//进程状态更新为执行
			pcb->update_state(ProcessState::Execute);
			pcb->timer_execute.on_update(delta);
		}
	}
}