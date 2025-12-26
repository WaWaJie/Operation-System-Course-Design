#include"8.1deadlock_prevention.h"


void DeadLockPrevention::on_enter()
{
	//str_tip = "";
	str_tip_list.clear();
	process_idx = 1;
	sum_time = 0;
	algo_choose_id = 0;
	current_pcb = nullptr;
	can_execute = false;
}

void DeadLockPrevention::on_exit()
{
	ConfigManager::instance()->clear_resource();
	pcb_queue_normal.clear();
}

void DeadLockPrevention::on_input(const SDL_Event* event)
{
	if (ImGui::GetIO().WantCaptureKeyboard)return;
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_e)
	{
		show_window_text = !show_window_text;
	}
}

void DeadLockPrevention::on_update(float delta)
{
	on_update_imgui_region1(delta);
	on_update_imgui_region2(delta);
	on_update_imgui_region3(delta);

	render_window_text();
	if (!can_execute)return;
	sum_time += delta;
	on_update_process(delta);
}

void DeadLockPrevention::on_update_imgui_region1(float delta)
{
	static const char* resource_name[] = { u8"ResourceA", u8"ResourceB", u8"ResourceC", u8"ResourceD" };
	static int resource_to_add_id = 0;

	ImGui::BeginChild("deadlock_prevention_region1", { ImGui::GetContentRegionAvail().x, 140 });

	// ==================== 第一部分：添加资源 ====================
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), u8"资源管理");
	ImGui::Separator();

	// 使用分组框包裹添加资源的部分
	ImGui::BeginGroup();
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(u8"添加资源:");
		ImGui::SameLine();

		// 资源选择
		ImGui::SetNextItemWidth(100);
		ImGui::Combo(u8"##deadlock_prevention_resource_name_to_choose", &resource_to_add_id, resource_name, 4);

		ImGui::SameLine();
		ImGui::Text(u8"数量:");
		ImGui::SameLine();

		// 数量输入
		ImGui::SetNextItemWidth(80);
		ImGui::InputInt(u8"##deadlock_prevention_resource_num_to_choose", &num_of_resource_to_add);

		ImGui::SameLine();
		// 添加按钮
		if (ImGui::Button(u8"添加", { 80, 24 }))
		{
			std::string rname = resource_name[resource_to_add_id];
			ConfigManager::instance()->add_resource(rname, num_of_resource_to_add);
			ConfigManager::instance()->add_resource_max(rname, num_of_resource_to_add);
			std::string new_str_tip = u8"系统资源 " + rname + u8" 增加了 " + std::to_string(num_of_resource_to_add) + u8" 个\n";
			str_tip_list.push_back(TextString(new_str_tip, TextType::Info));
		}
	}
	ImGui::EndGroup();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// ==================== 第二部分：进程控制按钮 ====================
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), u8"进程控制");
	ImGui::Separator();

	// 按钮组 - 使用更紧凑的布局
	ImGui::BeginGroup();
	{
		static bool show_process_add_window = false;

		// 创建按钮行
		ImGui::Columns(3, "ControlButtons", false);
		ImGui::SetColumnWidth(0, 110);
		ImGui::SetColumnWidth(1, 110);
		ImGui::SetColumnWidth(2, 110);

		// 第一列：添加进程按钮
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.9f, 0.9f));
		if (ImGui::Button(u8"添加进程", { 100, 28 }))
		{
			show_process_add_window = true;
		}
		render_window_process_add(show_process_add_window);
		ImGui::PopStyleColor(2);

		ImGui::NextColumn();

		// 第二列：开始执行按钮
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 0.9f));
		if (ImGui::Button(u8"开始执行", { 100, 28 }))
		{
			can_execute = true;
			std::string new_str_tip = u8"开始执行进程\n";
			str_tip_list.push_back(TextString(new_str_tip, TextType::Warning));
		}
		ImGui::PopStyleColor(2);

		ImGui::NextColumn();

		// 第三列：暂停执行按钮
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.2f, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.7f, 0.3f, 0.9f));
		if (ImGui::Button(u8"暂停执行", { 100, 28 }))
		{
			can_execute = false;
			std::string new_str_tip = u8"暂停执行进程\n";
			str_tip_list.push_back(TextString(new_str_tip, TextType::Warning));
		}
		ImGui::PopStyleColor(2);

		ImGui::Columns(1); // 恢复单列布局
	}
	ImGui::EndGroup();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// ==================== 第三部分：资源状态显示 ====================
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), u8"资源状态");
	ImGui::Separator();

	const auto& resource_pool = ConfigManager::instance()->get_resource_pool();
	float bar_height = 16.0f;

	if (!resource_pool.empty())
	{
		// 使用表格布局来显示资源状态
		if (ImGui::BeginTable("ResourceStatus", 4, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::TableSetupColumn(u8"图标", ImGuiTableColumnFlags_WidthFixed, 30);
			ImGui::TableSetupColumn(u8"名称", ImGuiTableColumnFlags_WidthFixed, 100);
			ImGui::TableSetupColumn(u8"数量", ImGuiTableColumnFlags_WidthFixed, 120);
			ImGui::TableSetupColumn(u8"进度", ImGuiTableColumnFlags_WidthStretch);

			for (auto& resource : resource_pool)
			{
				ImGui::TableNextRow();

				// 图标列
				ImGui::TableSetColumnIndex(0);
				ImGui::Image(ResourcesManager::instance()->find_texture("Resource"), { 20, 20 });

				// 名称列
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", resource.first.c_str());

				// 数量列
				ImGui::TableSetColumnIndex(2);
				const auto resource_number_max = ConfigManager::instance()->get_resource_number_max(resource.first);
				ImGui::Text("%d / %d", resource.second, (int)resource_number_max);

				// 进度条列
				ImGui::TableSetColumnIndex(3);
				float progress = (float)resource.second / std::max(resource_number_max, 1);

				// 根据进度值选择颜色
				if (progress > 0.7f)
					ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.6f, 0.2f, 0.8f)); // 绿色 - 充足
				else if (progress > 0.3f)
					ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.8f, 0.2f, 0.8f)); // 黄色 - 中等
				else
					ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.3f, 0.2f, 0.8f)); // 红色 - 紧张

				ImGui::ProgressBar(progress, ImVec2(-FLT_MIN, bar_height), "");
				ImGui::PopStyleColor();

				// 在进度条上显示百分比
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 45);
				ImGui::Text("%.0f%%", progress * 100);
			}
			ImGui::EndTable();
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), u8"当前没有资源可用");
		ImGui::SameLine();
		ImGui::TextDisabled(u8"(请先添加资源)");
	}

	ImGui::EndChild();
	ImGui::Separator();
}

void DeadLockPrevention::on_update_imgui_region2(float delta)
{
	ImGui::BeginChild("deadlock_prevention_region2", { ImGui::GetContentRegionAvail().x,400 });

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
			if (pcb->current_state == ProcessState::Finish) 
			{
				std::string new_str_tip = u8"WARNING: 选中进程 " + current_pcb->pname + u8"  当前进程已经运行结束\n";
                str_tip_list.push_back(TextString(new_str_tip, TextType::Warning));
			}
			else if (pcb->current_state != ProcessState::ActiveBlock)
			{
				current_pcb->update_state(ProcessState::ActiveBlock);
				std::string new_str_tip = u8"选中进程 " + current_pcb->pname + u8"进行手动阻断，进入活动阻塞状态\n";
				str_tip_list.push_back(TextString(new_str_tip, TextType::Warning));
				current_pcb->signal_and();
				std::string new_str_tip2 = u8"进程 " + current_pcb->pname + u8" 释放资源---"
					+ "\n\t\tResourceA : " + std::to_string(current_pcb->resource_hold["ResourceA"])
					+ "\n\t\tResourceB : " + std::to_string(current_pcb->resource_hold["ResourceB"])
					+ "\n\t\tResourceC : " + std::to_string(current_pcb->resource_hold["ResourceC"])
					+ "\n\t\tResourceD : " + std::to_string(current_pcb->resource_hold["ResourceD"]) + "\n";
				str_tip_list.push_back(TextString(new_str_tip2, TextType::Info));
			}
			else
			{
                current_pcb->update_state(ProcessState::ActiveReady);
				std::string new_str_tip = u8"选中进程 " + current_pcb->pname + u8"解除手动阻断，进入活动就绪状态\n";
				str_tip_list.push_back(TextString(new_str_tip, TextType::Warning));
			}
		}

		pcb_queue_normal[i]->draw_state({ 850,182 + height_delta * i }, 10);
	}

	ImGui::EndChild();
	ImGui::Separator();
}

void DeadLockPrevention::on_update_imgui_region3(float delta)
{
	ImGui::BeginChildFrame(1, { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y });
	ImGui::Button(u8"清空日志", { ImGui::GetContentRegionAvail().x,30 });
	if (ImGui::IsItemClicked())
	{
        str_tip_list.clear();
	}
	for (auto& text_info : str_tip_list)
	{
		// 安全跳过空文本
		if (text_info.text_info.empty())
			continue;
		ImGui::PushStyleColor(ImGuiCol_Text, static_cast<ImVec4>(ConfigManager::instance()->get_text_color(text_info.type)));
		ImGui::TextUnformatted(text_info.text_info.c_str());
		ImGui::PopStyleColor();
	}

	static int last_size = 0;
	if (last_size != str_tip_list.size() )
	{
		ImGui::SetScrollY(ImGui::GetScrollMaxY() + 2000);
	}
    last_size = str_tip_list.size();
	
	ImGui::EndChildFrame();
}

void DeadLockPrevention::on_update_process(float delta)
{
	for (auto& pcb : pcb_queue_normal)
	{
		if (pcb->current_state == ProcessState::Execute)//执行态		
		{
			pcb->timer_execute.on_update(delta);
			if (pcb->current_state == ProcessState::Finish)
			{
  				std::string new_str_tip = u8"进程 " + pcb->pname + u8" 执行完毕，进入完成态\n";
				str_tip_list.push_back(TextString(new_str_tip, TextType::Finish));
				std::string new_str_tip2 = u8"进程 " + pcb->pname + u8" 释放资源---"
					+ "\n\t\tResourceA : " + std::to_string(pcb->resource_hold["ResourceA"])
					+ "\n\t\tResourceB : " + std::to_string(pcb->resource_hold["ResourceB"])
					+ "\n\t\tResourceC : " + std::to_string(pcb->resource_hold["ResourceC"])
					+ "\n\t\tResourceD : " + std::to_string(pcb->resource_hold["ResourceD"]) + "\n";
                str_tip_list.push_back(TextString(new_str_tip2, TextType::Info));
				pcb->signal_and();
			}
		}
		else if (pcb->current_state == ProcessState::ActiveReady)//活动就绪态
		{
			pcb->wait_and(resource_name_list);
			if (pcb->is_resources_ok())
			{
				pcb->update_state(ProcessState::Execute);
				std::string new_str_tip = u8"进程 " + pcb->pname + u8" 资源满足，进入执行态\n";
				str_tip_list.push_back(TextString(new_str_tip, TextType::Info));
			}
		}
	}
}

void DeadLockPrevention::render_window_text()
{
	if (!show_window_text)return;
	ImGui::SetNextWindowSize({ 600, 300 });
	if (ImGui::Begin(u8"预防死锁系统说明", &show_window_text))
	{
		ImGui::BeginChild("DeadlockPreventionTipContent", ImVec2(0, 0), true);
		{
			ImGui::PushTextWrapPos(0.0f);

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

void DeadLockPrevention::render_window_process_add(bool& flag)
{
	if (!flag) return;

	// 设置窗口属性 - 居中显示，首次使用设置大小
	ImGui::SetNextWindowSize({ 450, 400 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

	if (ImGui::Begin(u8"添加进程资源配置", &flag, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), u8"进程资源需求配置");
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text(u8"资源需求设置：");
		ImGui::Spacing();

		ImGui::BeginGroup();
		ImGui::PushItemWidth(180);

		ImGui::Columns(3, "resource_table", false);
		ImGui::SetColumnWidth(0, 80);
		ImGui::SetColumnWidth(1, 150);
		ImGui::SetColumnWidth(2, 150);

		ImGui::Text(u8"资源类型"); ImGui::NextColumn();
		ImGui::Text(u8"需求数量"); ImGui::NextColumn();
		ImGui::Text(u8"资源ID"); ImGui::NextColumn();
		ImGui::Separator();

		// 四个资源的配置行
		const char* resource_names[] = { u8"ResourceA", u8"ResourceB", u8"ResourceC", u8"ResourceD" };
		const ImVec4 row_colors[] = {
			ImVec4(0.95f, 0.95f, 0.95f, 1.0f),  // 浅灰
			ImVec4(0.90f, 0.90f, 0.90f, 1.0f),  // 中灰
			ImVec4(0.95f, 0.95f, 0.95f, 1.0f),  // 浅灰
			ImVec4(0.90f, 0.90f, 0.90f, 1.0f)   // 中灰
		};

		for (int i = 0; i < 4; i++)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Button, row_colors[i]);

			ImGui::Text(resource_names[i]);
			ImGui::NextColumn();

			std::string slider_label = "##req_" + std::to_string(i);
			ImGui::SliderInt(slider_label.c_str(), &num_of_resource_process_request[i], 0, 10, u8"数量: %d");
			ImGui::NextColumn();

			std::string id_label = "##id_" + std::to_string(i);
			ImGui::SliderInt(id_label.c_str(), &id_of_resource_process_request[i], 0, 10, u8"ID: %d");
			ImGui::NextColumn();

			ImGui::PopStyleColor(2);
		}

		ImGui::Columns(1);
		ImGui::PopItemWidth();
		ImGui::EndGroup();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// 按钮区域 - 居中显示
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 220) * 0.5f);

		if (ImGui::Button(u8"确认添加", ImVec2(100, 35)))
		{
			if (pcb_queue_normal.size() >= 8)
			{
				std::string new_str_tip = u8"系统最多允许存在8个进程，无法创建新进程\n";
				str_tip_list.push_back(TextString(new_str_tip, TextType::Error));
			}
			else
			{
				auto pcb = std::make_shared<PCB>();
				pcb->time_execute = process_time;
				pcb->pname = u8"进程" + std::to_string(process_idx++);
				pcb->update_state(ProcessState::ActiveReady);

				for (int i = 0; i < 4; i++)
				{
					pcb->task_list.push_back(std::make_shared<ProcessResource>(
						resource_names[i],
						id_of_resource_process_request[i],
						num_of_resource_process_request[i]
					));
					pcb->resource_max[resource_names[i]] = num_of_resource_process_request[i];
					pcb->resource_hold[resource_names[i]] = 0;
				}
				std::string new_str_tip = u8"进行序号排序\n";
                str_tip_list.push_back(TextString(new_str_tip, TextType::KeyInfo));
				sort(pcb->task_list.begin(), pcb->task_list.end(), [](const auto& a, const auto& b) { return a->id < b->id; });
				for (int i = 0; i < pcb->task_list.size(); i++)
				{
					std::string new_str_tip = u8"进程 " + pcb->pname + u8" 需要资源 "
						+ pcb->task_list[i]->name + u8" 数量: " + std::to_string(pcb->task_list[i]->need_num)
						+ u8" 资源ID: " + std::to_string(pcb->task_list[i]->id) + u8"\n";
					str_tip_list.push_back(TextString(new_str_tip, TextType::Info));
				}

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

		ImGui::SameLine();

		// 取消按钮
		if (ImGui::Button(u8"取消", ImVec2(100, 35)))
		{
			flag = false;  
			memset(num_of_resource_process_request, 0, sizeof(num_of_resource_process_request));
			memset(id_of_resource_process_request, 0, sizeof(id_of_resource_process_request));
		}

	}

	ImGui::End();
}

