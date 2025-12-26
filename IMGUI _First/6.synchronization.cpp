#include"6.synchronization.h"

//timer_execute

void Synchronization::on_enter()
{
	str_tip_list.clear();
	process_idx = 1;
	sum_time = 0;
	algo_choose_id = 0;
	current_pcb = nullptr;
	can_execute = false;

	pcb_idx_execute = 0;
	resource_occupy = 1;  // 默认值
	process_time = 1.0f;  // 默认值
	resource_to_add = 1;  // 默认值
}

void Synchronization::on_exit()
{
	ConfigManager::instance()->clear_resource();
	pcb_queue_normal.clear();
	pcb_queue_can_execute.clear();
	str_tip_list.clear();
}

void Synchronization::on_update(float delta)
{
	on_update_imgui_region1(delta);
	on_update_imgui_region2(delta);
	on_update_imgui_region3(delta);
	if (!can_execute)return;
	sum_time += delta;
	on_update_process(delta);
}

void Synchronization::on_update_imgui_region1(float delta)
{
	static const char* items[] = { "Resource_A", "Resource_B", "Resource_C" ,"Resource_D" ,"Resource_E" ,"Resource_F" ,"Resource_G" ,"Resource_H" };
	static int item_current = 0;

	ImGui::BeginChild("synchronization_region1", { ImGui::GetContentRegionAvail().x,120 });

	if (current_pcb)
	{
		// 进程信息显示行，优化间距
		ImGui::Text(u8"选择进程: %s", current_pcb->pname.c_str());
		ImGui::SameLine();
		ImGui::Text(u8"剩余时间: %.1f s", current_pcb->timer_execute.get_remain_time());
		ImGui::SameLine();
		ImGui::Text(u8"占用资源: %s  数量: %d",
			current_pcb->resource_occupied_name.c_str(),
			current_pcb->resource_hold[current_pcb->resource_occupied_name]);
		ImGui::SameLine();
		ImGui::Text(u8"当前状态: %s", current_pcb->state_name.c_str());
		current_pcb->draw_state({ 750,26 }, 5);

		// 创建可折叠的资源设置窗口（使用ImGui原生折叠功能）
		{
			// 保留原有窗口尺寸逻辑，移除自定义折叠变量
			ImGui::SetNextWindowSize({ 320, 220 }, ImGuiCond_Once);
			// 启用原生折叠功能，移除NoCollapse标志
			std::string title_str = current_pcb->pname + u8" - 占用资源设置";
			ImGui::Begin(title_str.c_str(), nullptr, ImGuiWindowFlags_NoResize);

			// 优化布局间距
			ImGui::Spacing();

			// 资源名称下拉列表，添加标签
			ImGui::Text(u8"选择资源:");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200);
			if (ImGui::Combo("##synchronization_current_resource_name_listbox",
				&item_current, items, IM_ARRAYSIZE(items), 4))
			{
				strcpy_s(resource_name, items[item_current]);
				strcpy_s(resource_occupied_name, items[item_current]);
			}

			ImGui::Spacing();
			// 突出显示当前占用量
			ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f),
				u8"当前所需占用量: %d", current_pcb->resource_max[resource_occupied_name]);

			ImGui::Separator();
			ImGui::Spacing();

			// 修改数量区域布局优化
			static int modify_amount = 0; // 保留原有静态变量
			ImGui::Text(u8"修改数量:");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::InputInt("##modify_resource_occupied_amount_input", &modify_amount, 1);
			ImGui::SameLine();

			// 美化确认按钮
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.2f, 0.8f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));
			if (ImGui::Button(u8"确认修改", ImVec2(80, 28)))
			{
				int available_resource = ConfigManager::instance()->get_resource_number(current_pcb->resource_occupied_name);
				current_pcb->resource_max[resource_occupied_name] += modify_amount;
				modify_amount = 0; // 重置输入
			}
			ImGui::PopStyleColor(3);

			ImGui::Spacing();
			ImGui::Text(u8"资源需求列表:");
			ImGui::Indent(10);

			// 资源列表优化显示
			for (auto& res : current_pcb->resource_max)
			{
				ImGui::Image(ResourcesManager::instance()->find_texture("Resource"), { 20,20 });
				ImGui::SameLine();
				ImGui::Text(u8"资源: %s  需求数量: %d", res.first.c_str(), res.second);
				ImGui::Spacing();
			}
			ImGui::Unindent(10);

			ImGui::End();
		}
	}
	ImGui::BeginGroup();
	ImGui::RadioButton(u8"记录型信号量", &algo_choose_id, 0); ImGui::SameLine();
	ImGui::RadioButton(u8"AND型信号量", &algo_choose_id, 1);
	ImGui::EndGroup();

	ImGui::Text(u8"资源名称: "); ImGui::SameLine();

	ImGui::SetNextItemWidth(150);

	if (ImGui::Combo("##synchronization_resource_name_listbox", &item_current, items, IM_ARRAYSIZE(items), 4))
	{
		strcpy_s(resource_name, items[item_current]);
		strcpy_s(resource_occupied_name, items[item_current]);
	}ImGui::SameLine();

	ImGui::Text(u8"资源量: "); ImGui::SameLine();
	ImGui::SetNextItemWidth(150);
	ImGui::InputInt("##synchronization_resource_amount_input", &resource_to_add);
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
	if (ImGui::Combo("##synchronization_resource_occupied_name_listbox", &item_current, items, IM_ARRAYSIZE(items), 4))
	{
		strcpy_s(resource_name, items[item_current]);
		strcpy_s(resource_occupied_name, items[item_current]);
	} ImGui::SameLine();
	ImGui::Text(u8"占用资源量:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	ImGui::SliderInt("##synchronization_resource_occupy", &resource_occupy, 0, 10, "%d"); ImGui::SameLine();
	ImGui::Text(u8"处理时间:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	ImGui::SliderFloat("##synchronization_process_time", &process_time, 0.1f, 5.0f, "%.1f"); ImGui::SameLine();

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
			pcb->can_wait = true;
			pcb->time_execute = process_time;
			pcb->pname = u8"进程" + std::to_string(process_idx++);
			pcb->update_state(ProcessState::ActiveBlock);

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
					sp->update_state(ProcessState::Finish);
					std::string tip = std::to_string(sum_time) + u8"---进程结束:  " + sp->pname;
					str_tip_list.push_back(TextString(tip, TextType::Info));
				});

			std::string tip = std::to_string(sum_time) + u8"---进程创建:  " + pcb->pname;
			str_tip_list.push_back(TextString(tip, TextType::Info));
			pcb_queue_normal.push_back(pcb);
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

void Synchronization::on_update_imgui_region2(float delta)
{
	ImGui::BeginChild("synchronization_region2", { ImGui::GetContentRegionAvail().x,400 });

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

void Synchronization::on_update_imgui_region3(float delta)
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

void Synchronization::on_update_process(float delta)
{
	switch (algo_choose_id)
	{
	case 0:
		on_update_algo_record(delta);
		break;
	case 1:
		on_update_algo_and(delta);
		break;
	default:
		break;
	}
}

/*
1.空闲让进（必然满足）
2.忙则等待（必然满足）
3.有限等待（无可执行进程则释放所有资源）
4.让权等待（无可执行进程则释放所有资源）
*/

void Synchronization::on_update_algo_record(float delta)
{
	// 1. 更新可执行队列中的当前执行进程
	if (!pcb_queue_can_execute.empty())
	{
		auto& pcb = pcb_queue_can_execute[pcb_idx_execute];

		// 如果进程是就绪状态，开始执行
		if (pcb->current_state == ProcessState::ActiveReady)
		{
			pcb->update_state(ProcessState::Execute);
			std::string tip = std::to_string(sum_time) + u8"---进程开始执行:  " + pcb->pname;
			str_tip_list.push_back(TextString(tip, TextType::Info));
		}

		// 更新进程的定时器
		pcb->timer_execute.on_update(delta);

		// 检查进程是否完成
		if (pcb->current_state == ProcessState::Finish)
		{
			// 进程完成，释放资源
			for (auto& resource : pcb->resource_hold)
			{
				if (resource.second > 0)
				{
					pcb->signal_record(resource.first);
				}
			}
			std::string tip = std::to_string(sum_time) + u8"---进程完成:  " + pcb->pname;
			str_tip_list.push_back(TextString(tip, TextType::Info));

			// 从可执行队列中移除
			pcb_queue_can_execute.erase(pcb_queue_can_execute.begin() + pcb_idx_execute);

			// 调整索引
			if (pcb_queue_can_execute.empty())
			{
				pcb_idx_execute = 0;
			}
			else
			{
				pcb_idx_execute = pcb_idx_execute % pcb_queue_can_execute.size();
			}
		}
		// 检查时间片是否用完 (0.1s)
		else if (pcb->timer_time_tick.get_remain_time() <= 0)
		{
			// 时间片用完，进入阻塞状态但保持已分配的资源
			pcb->update_state(ProcessState::ActiveBlock);
			std::string tip = std::to_string(sum_time) + u8"---时间片用完:  " + pcb->pname + u8" 进入阻塞状态，保持已分配资源";
			str_tip_list.push_back(TextString(tip, TextType::Info));

			// 启动等待资源的最长时间计时器
			pcb->timer_to_wait_max.restart();

			// 从可执行队列中移除当前进程
			pcb_queue_can_execute.erase(pcb_queue_can_execute.begin() + pcb_idx_execute);

			// 切换到下一个进程
			if (!pcb_queue_can_execute.empty())
			{
				pcb_idx_execute = pcb_idx_execute % pcb_queue_can_execute.size();

				// 立即开始执行下一个进程
				auto& next_pcb = pcb_queue_can_execute[pcb_idx_execute];
				if (next_pcb->current_state == ProcessState::ActiveReady)
				{
					next_pcb->update_state(ProcessState::Execute);
					next_pcb->timer_time_tick.restart();
					std::string tip = std::to_string(sum_time) + u8"---切换到进程:  " + next_pcb->pname;
					str_tip_list.push_back(TextString(tip, TextType::Info));
				}
			}
			else
			{
				pcb_idx_execute = 0;
			}
		}
	}

	// 2. 更新所有阻塞进程的状态
	for (auto& pcb : pcb_queue_normal)
	{
		// 跳过已完成和正在执行的进程
		if (pcb->current_state == ProcessState::Finish ||
			pcb->current_state == ProcessState::Execute)
		{
			continue;
		}

		// 处理阻塞状态的进程
		if (pcb->current_state == ProcessState::ActiveBlock)
		{
			// 检查进程是否已经满足所有资源需求
			if (pcb->is_resources_ok())
			{
				// 资源已满足，加入就绪队列
				pcb->update_state(ProcessState::ActiveReady);
				pcb_queue_can_execute.push_back(pcb);
				std::string tip = std::to_string(sum_time) + u8"---资源已满足:  " + pcb->pname + u8" 进入就绪队列";
				str_tip_list.push_back(TextString(tip, TextType::Info));

				// 如果没有正在执行的进程，立即开始执行这个新就绪的进程
				if (pcb_queue_can_execute.size() == 1) // 只有这一个就绪进程
				{
					pcb_idx_execute = 0;
					pcb->update_state(ProcessState::Execute);
					pcb->timer_time_tick.restart();
					std::string tip = std::to_string(sum_time) + u8"---开始执行:  " + pcb->pname;
					str_tip_list.push_back(TextString(tip, TextType::Info));
				}
			}
			else
			{
				// 资源不满足，尝试分配更多资源
				bool allocated = false;
				for (auto& resource : pcb->resource_max)
				{
					int needed = resource.second - pcb->resource_hold[resource.first];
					if (needed > 0 && ConfigManager::instance()->get_resource_number(resource.first) >= needed)
					{
						// 分配资源
						pcb->wait_record(resource.first);
						allocated = true;
						std::string tip = std::to_string(sum_time) + u8"---分配资源:  " + pcb->pname +
							u8" 获得 " + resource.first + u8" 数量: " + std::to_string(needed);
						str_tip_list.push_back(TextString(tip, TextType::Info));
					}
				}

				// 如果分配了资源，检查是否已满足所有需求
				if (allocated && pcb->is_resources_ok())
				{
					pcb->update_state(ProcessState::ActiveReady);
					pcb_queue_can_execute.push_back(pcb);
					std::string tip = std::to_string(sum_time) + u8"---资源已满足:  " + pcb->pname + u8" 进入就绪队列";
					str_tip_list.push_back(TextString(tip, TextType::Info));

					// 如果没有正在执行的进程，立即开始执行
					if (pcb_queue_can_execute.size() == 1)
					{
						pcb_idx_execute = 0;
						pcb->update_state(ProcessState::Execute);
						pcb->timer_time_tick.restart();
						std::string tip = std::to_string(sum_time) + u8"---开始执行:  " + pcb->pname;
						str_tip_list.push_back(TextString(tip, TextType::Info));
					}
				}
				else
				{
					// 资源仍然不满足，更新等待计时器
					pcb->timer_to_wait_max.on_update(delta);

					// 等待超时，释放已持有的资源
					if (pcb->timer_to_wait_max.get_remain_time() <= 0)
					{
						bool released = false;
						for (auto& resource : pcb->resource_hold)
						{
							if (resource.second > 0)
							{
								pcb->signal_record(resource.first);
								released = true;
							}
						}
						if (released)
						{
							std::string tip = std::to_string(sum_time) + u8"---等待超时释放资源:  " + pcb->pname;
							str_tip_list.push_back(TextString(tip, TextType::Warning));
						}

						// 设置进程暂时不参与资源分配
						pcb->can_wait = false;
						pcb->timer_to_ask_for_resource.restart();
						pcb->timer_to_wait_max.restart();
					}
				}
			}
		}

		// 处理暂时不能参与资源分配的进程 (can_wait = false)
		if (!pcb->can_wait && pcb->current_state == ProcessState::ActiveBlock)
		{
			// 更新重新申请资源的计时器
			pcb->timer_to_ask_for_resource.on_update(delta);

			// 可以重新参与资源分配
			if (pcb->timer_to_ask_for_resource.get_remain_time() <= 0)
			{
				pcb->can_wait = true;
				pcb->timer_to_wait_max.restart(); // 重启等待计时器
				std::string tip = std::to_string(sum_time) + u8"---进程重新参与资源分配:  " + pcb->pname;
				str_tip_list.push_back(TextString(tip, TextType::Info));
			}
		}
	}

	// 3. 如果没有进程在执行，但可执行队列不为空，开始执行下一个进程
	if (!pcb_queue_can_execute.empty() && pcb_idx_execute < pcb_queue_can_execute.size())
	{
		auto& next_pcb = pcb_queue_can_execute[pcb_idx_execute];
		if (next_pcb->current_state == ProcessState::ActiveReady)
		{
			next_pcb->update_state(ProcessState::Execute);
			next_pcb->timer_time_tick.restart();
			std::string tip = std::to_string(sum_time) + u8"---开始执行:  " + next_pcb->pname;
			str_tip_list.push_back(TextString(tip, TextType::Info));
		}
	}
}

//只要当前资源满足所有请求就全部分配，否则都不分配
void Synchronization::on_update_algo_and(float delta)
{
	// 1. 更新可执行队列中的当前执行进程
	if (!pcb_queue_can_execute.empty())
	{
		auto& pcb = pcb_queue_can_execute[pcb_idx_execute];

		// 如果进程是就绪状态，开始执行
		if (pcb->current_state == ProcessState::ActiveReady)
		{
			pcb->update_state(ProcessState::Execute);
			pcb->timer_time_tick.restart(); // 重启时间片计时器
			std::string tip = std::to_string(sum_time) + u8"---进程开始执行:  " + pcb->pname;
			str_tip_list.push_back(TextString(tip, TextType::Info));
		}

		// 更新进程的定时器
		pcb->timer_time_tick.on_update(delta);
		pcb->timer_execute.on_update(delta);

		// 检查进程是否完成
		if (pcb->current_state == ProcessState::Finish)
		{
			// 进程完成，使用AND型信号量释放所有资源
			pcb->signal_and();
			std::string tip = std::to_string(sum_time) + u8"---进程完成:  " + pcb->pname;
			str_tip_list.push_back(TextString(tip, TextType::Info));

			// 从可执行队列中移除
			pcb_queue_can_execute.erase(pcb_queue_can_execute.begin() + pcb_idx_execute);

			// 调整索引
			if (pcb_queue_can_execute.empty())
			{
				pcb_idx_execute = 0;
			}
			else
			{
				pcb_idx_execute = pcb_idx_execute % pcb_queue_can_execute.size();
			}
		}
		// 检查时间片是否用完 (0.1s)
		else if (pcb->timer_time_tick.get_remain_time() <= 0)
		{
			// 时间片用完，进入阻塞状态但保持已分配的资源
			pcb->update_state(ProcessState::ActiveBlock);
			std::string tip = std::to_string(sum_time) + u8"---时间片用完:  " + pcb->pname + u8" 进入阻塞状态，保持已分配资源";
			str_tip_list.push_back(TextString(tip, TextType::Info));

			// 启动等待资源的最长时间计时器
			pcb->timer_to_wait_max.restart();

			// 从可执行队列中移除当前进程
			pcb_queue_can_execute.erase(pcb_queue_can_execute.begin() + pcb_idx_execute);

			// 切换到下一个进程
			if (!pcb_queue_can_execute.empty())
			{
				pcb_idx_execute = pcb_idx_execute % pcb_queue_can_execute.size();

				// 立即开始执行下一个进程
				auto& next_pcb = pcb_queue_can_execute[pcb_idx_execute];
				if (next_pcb->current_state == ProcessState::ActiveReady)
				{
					next_pcb->update_state(ProcessState::Execute);
					next_pcb->timer_time_tick.restart();
					std::string tip = std::to_string(sum_time) + u8"---切换到进程:  " + next_pcb->pname;
					str_tip_list.push_back(TextString(tip, TextType::Info));
				}
			}
			else
			{
				pcb_idx_execute = 0;
			}
		}
	}

	// 2. 更新所有阻塞进程的状态
	for (auto& pcb : pcb_queue_normal)
	{
		// 跳过已完成和正在执行的进程
		if (pcb->current_state == ProcessState::Finish ||
			pcb->current_state == ProcessState::Execute)
		{
			continue;
		}

		// 处理阻塞状态的进程
		if (pcb->current_state == ProcessState::ActiveBlock)
		{
			// 检查进程是否已经满足所有资源需求
			if (pcb->is_resources_ok())
			{
				// 资源已满足，加入就绪队列
				pcb->update_state(ProcessState::ActiveReady);
				pcb_queue_can_execute.push_back(pcb);
				std::string tip = std::to_string(sum_time) + u8"---资源已满足:  " + pcb->pname + u8" 进入就绪队列";
				str_tip_list.push_back(TextString(tip, TextType::Info));

				// 如果没有正在执行的进程，立即开始执行这个新就绪的进程
				if (pcb_queue_can_execute.size() == 1) // 只有这一个就绪进程
				{
					pcb_idx_execute = 0;
					pcb->update_state(ProcessState::Execute);
					pcb->timer_time_tick.restart();
					std::string tip = std::to_string(sum_time) + u8"---开始执行:  " + pcb->pname;
					str_tip_list.push_back(TextString(tip, TextType::Info));
				}
			}
			else
			{
				// 对于AND型信号量，要么获得所有资源，要么一个都不获得
				// 收集所有需要的资源名称
				std::vector<std::string> needed_resources;
				for (auto& resource : pcb->resource_max)
				{
					if (resource.second > pcb->resource_hold[resource.first])
					{
						needed_resources.push_back(resource.first);
					}
				}

				// 尝试一次性获取所有需要的资源
				if (!needed_resources.empty())
				{
					pcb->wait_and(needed_resources);

					// 检查是否成功获取了所有资源
					if (pcb->is_resources_ok())
					{
						pcb->update_state(ProcessState::ActiveReady);
						pcb_queue_can_execute.push_back(pcb);
						std::string tip = std::to_string(sum_time) + u8"---AND型资源分配成功:  " + pcb->pname + u8" 进入就绪队列";
						str_tip_list.push_back(TextString(tip, TextType::Info));

						// 如果没有正在执行的进程，立即开始执行
						if (pcb_queue_can_execute.size() == 1)
						{
							pcb_idx_execute = 0;
							pcb->update_state(ProcessState::Execute);
							pcb->timer_time_tick.restart();
							std::string tip = std::to_string(sum_time) + u8"---开始执行:  " + pcb->pname;
							str_tip_list.push_back(TextString(tip, TextType::Info));
						}
					}
					else
					{
						// 资源分配失败，更新等待计时器
						pcb->timer_to_wait_max.on_update(delta);

						// 等待超时，释放已持有的资源
						if (pcb->timer_to_wait_max.get_remain_time() <= 0)
						{
							// 使用AND型信号量释放所有资源
							pcb->signal_and();
							std::string tip = std::to_string(sum_time) + u8"---等待超时释放资源:  " + pcb->pname;
							str_tip_list.push_back(TextString(tip, TextType::Warning));

							// 设置进程暂时不参与资源分配
							pcb->can_wait = false;
							pcb->timer_to_ask_for_resource.restart();
							pcb->timer_to_wait_max.restart();
						}
					}
				}
			}
		}

		// 处理暂时不能参与资源分配的进程 (can_wait = false)
		if (!pcb->can_wait && pcb->current_state == ProcessState::ActiveBlock)
		{
			// 更新重新申请资源的计时器
			pcb->timer_to_ask_for_resource.on_update(delta);

			// 可以重新参与资源分配
			if (pcb->timer_to_ask_for_resource.get_remain_time() <= 0)
			{
				pcb->can_wait = true;
				pcb->timer_to_wait_max.restart(); // 重启等待计时器
				std::string tip = std::to_string(sum_time) + u8"---进程重新参与资源分配:  " + pcb->pname;
				str_tip_list.push_back(TextString(tip, TextType::Info));
			}
		}
	}

	// 3. 如果没有进程在执行，但可执行队列不为空，开始执行下一个进程
	if (!pcb_queue_can_execute.empty() && pcb_idx_execute < pcb_queue_can_execute.size())
	{
		auto& next_pcb = pcb_queue_can_execute[pcb_idx_execute];
		if (next_pcb->current_state == ProcessState::ActiveReady)
		{
			next_pcb->update_state(ProcessState::Execute);
			next_pcb->timer_time_tick.restart();
			std::string tip = std::to_string(sum_time) + u8"---开始执行:  " + next_pcb->pname;
			str_tip_list.push_back(TextString(tip, TextType::Info));
		}
	}
}