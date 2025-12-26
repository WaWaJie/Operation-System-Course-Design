#include "8.3deadlock_check_and_release.h"

void DeadLockCheckAndRelease::on_enter()
{

    str_tip_list.clear();
    process_idx = 1;
    sum_time = 0;
    current_pcb = nullptr;
    can_execute = false;
    deadlock_detected = false;

    // 初始化数据结构
    resource_names = { "Resource_A", "Resource_B", "Resource_C", "Resource_D" };
    available = { 0,0,0,0 };

    ConfigManager::instance()->clear_resource();
    pcb_queue_normal.clear();

    // 初始化资源
    for (int i = 0; i < resource_names.size(); i++) {
        ConfigManager::instance()->add_resource(resource_names[i], available[i]);
        ConfigManager::instance()->add_resource_max(resource_names[i], available[i]);
    }

    str_tip_list.push_back(TextString(u8"死锁检测与解除模块已启动", TextType::Info));
    str_tip_list.push_back(TextString(u8"简化版：检测到死锁时终止所有进程", TextType::Info));
}

void DeadLockCheckAndRelease::on_exit()
{
    ConfigManager::instance()->clear_resource();
    pcb_queue_normal.clear();
    str_tip_list.clear();
}

void DeadLockCheckAndRelease::on_input(const SDL_Event* event)
{
    if (ImGui::GetIO().WantCaptureKeyboard) return;
    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_d) {
        detect_deadlock();
    }
}

void DeadLockCheckAndRelease::on_update(float delta)
{
    on_update_imgui_region1(delta);
    on_update_imgui_region2(delta);
    on_update_imgui_region3(delta);
    on_update_imgui_region4(delta);

    if (!can_execute) return;
    sum_time += delta;
}

void DeadLockCheckAndRelease::on_update_imgui_region1(float delta)
{
    // 资源管理区域
    ImGui::BeginChild("deadlock_check_region1", { ImGui::GetContentRegionAvail().x, 80 });

    ImGui::Text(u8"资源管理");
    ImGui::Separator();

    ImGui::Text(u8"资源类型:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    const char* resource_types[] = { "Resource_A", "Resource_B", "Resource_C", "Resource_D" };
    ImGui::Combo("##resource_type", &selected_resource_type, resource_types, IM_ARRAYSIZE(resource_types));

    ImGui::SameLine();
    ImGui::Text(u8"数量:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::InputInt("##resource_amount", &num_of_resource_to_add, 1, 10);

    ImGui::SameLine();
    if (ImGui::Button(u8"添加资源"))
    {
        std::string resource_name = resource_types[selected_resource_type];
        ConfigManager::instance()->add_resource(resource_name, num_of_resource_to_add);
        ConfigManager::instance()->add_resource_max(resource_name, num_of_resource_to_add);

        available[selected_resource_type] += num_of_resource_to_add;

        std::string tip = std::to_string(sum_time) + u8"---添加资源: " + resource_name +
            u8" 数量: " + std::to_string(num_of_resource_to_add);
        str_tip_list.push_back(TextString(tip, TextType::Info));
    }

    ImGui::SameLine();
    if (ImGui::Button(u8"删除资源"))
    {
        std::string resource_name = resource_types[selected_resource_type];
        int current_amount = ConfigManager::instance()->get_resource_number(resource_name);
        int to_remove = std::min(num_of_resource_to_add, current_amount);

        ConfigManager::instance()->sub_resource(resource_name, to_remove);
        ConfigManager::instance()->sub_resource_max(resource_name, to_remove);

        available[selected_resource_type] = std::max(0, available[selected_resource_type] - to_remove);

        std::string tip = std::to_string(sum_time) + u8"---删除资源: " + resource_name +
            u8" 数量: " + std::to_string(to_remove);
        str_tip_list.push_back(TextString(tip, TextType::Info));
    }

    ImGui::EndChild();
    ImGui::Separator();
}

void DeadLockCheckAndRelease::on_update_imgui_region2(float delta)
{
    // 进程管理区域
    ImGui::BeginChild("deadlock_check_region2", { ImGui::GetContentRegionAvail().x * 0.6f, 300 });

    ImGui::Text(u8"进程管理");
    ImGui::Separator();

    // 创建进程按钮
    if (ImGui::Button(u8"创建新进程", { ImGui::GetContentRegionAvail().x, 30 }))
    {
        if (pcb_queue_normal.size() >= 6)
        {
            std::string tip = std::to_string(sum_time) + u8"---进程创建失败: 进程数已达到最大值";
            str_tip_list.push_back(TextString(tip, TextType::Error));
        }
        else
        {
            auto pcb = std::make_shared<PCB>();
            pcb->can_wait = true;
            pcb->time_execute = 2.0;
            pcb->pname = u8"Process" + std::to_string(process_idx++);
            pcb->update_state(ProcessState::ActiveReady);

            pcb->timer_time_tick.set_one_shot(false);
            pcb->timer_execute.set_wait_time(2.0);
            pcb->timer_execute.set_one_shot(true);

            auto sp = pcb;
            pcb->timer_execute.set_on_timeout([&, sp]()
                {
                    sp->update_state(ProcessState::Finish);
                    std::string tip = std::to_string(sum_time) + u8"---进程结束:  " + sp->pname;
                    str_tip_list.push_back(TextString(tip, TextType::Info));

                    // 释放进程持有的所有资源
                    for (auto& resource : sp->resource_hold) {
                        ConfigManager::instance()->add_resource(resource.first, resource.second);
                        int resource_idx = std::distance(resource_names.begin(),
                            std::find(resource_names.begin(), resource_names.end(), resource.first));
                        if (resource_idx < available.size()) {
                            available[resource_idx] += resource.second;
                        }
                    }
                });

            std::string tip = std::to_string(sum_time) + u8"---进程创建:  " + pcb->pname;
            str_tip_list.push_back(TextString(tip, TextType::Info));
            pcb_queue_normal.push_back(pcb);
        }
    }

   

    // 进程列表显示
	on_update_process_resource(delta);


    ImGui::EndChild();
    ImGui::SameLine();

    // 系统状态区域
    ImGui::BeginChild("system_status", { 0, 300 }, true);
    ImGui::Text(u8"系统状态");
    ImGui::Separator();

    // 死锁检测状态
    ImGui::Text(u8"死锁检测状态:");
    ImGui::SameLine();
    if (deadlock_detected) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(u8"检测到死锁");
        ImGui::PopStyleColor();
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::Text(u8"系统正常");
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    // 资源状态
    ImGui::Text(u8"资源状态:");
    for (int i = 0; i < resource_names.size(); i++)
    {
        int current_amount = ConfigManager::instance()->get_resource_number(resource_names[i]);
        int max_amount = ConfigManager::instance()->get_resource_number_max(resource_names[i]);

        ImGui::Image(ResourcesManager::instance()->find_texture("Resource"), { 20,20 });
        ImGui::SameLine();
        ImGui::Text(u8"%s: %d/%d", resource_names[i].c_str(), current_amount, max_amount);
        ImGui::SameLine();
        ImGui::Text(u8"可用: %d", available[i]);

        float progress = max_amount > 0 ? (float)current_amount / max_amount : 0.0f;
        ImGui::ProgressBar(progress, { ImGui::GetContentRegionAvail().x, 20 });
    }

    ImGui::EndChild();
    ImGui::Separator();
}

void DeadLockCheckAndRelease::on_update_imgui_region3(float delta)
{
    // 死锁检测控制区域
    ImGui::BeginChild("deadlock_check_region3", { ImGui::GetContentRegionAvail().x, 80 });

    ImGui::Text(u8"死锁检测控制");
    ImGui::Separator();

    if (ImGui::Button(u8"开始执行", { 100, 30 }))
    {
        if (!can_execute)
        {
            str_tip_list.push_back(TextString(u8"开始执行（如果存在死锁，那么将终止所有进程）", TextType::Info));
        }
        can_execute = true;
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"立即检测", { 100, 30 }))
    {
		detect_deadlock();
        if (deadlock_detected)
        {
            std::string tip = std::to_string(sum_time) + u8"---检测到死锁!";
            str_tip_list.push_back(TextString(tip, TextType::Warning));
        }
        else
        {
            std::string tip = std::to_string(sum_time) + u8"---系统正常，未检测到死锁";
            str_tip_list.push_back(TextString(tip, TextType::Info));
        }
    }

    ImGui::SameLine();

    if (deadlock_detected && ImGui::Button(u8"终止所有进程", { 150, 30 }))
    {
        release_all_processes();
		std::string tip = std::to_string(sum_time) + u8"---所有进程已终止，死锁已解除";
        str_tip_list.push_back(TextString(tip, TextType::Info));
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"清空日志", { 100, 30 }))
    {
        str_tip_list.clear();
    }

    ImGui::EndChild();
    ImGui::Separator();
}

void DeadLockCheckAndRelease::on_update_imgui_region4(float delta)
{
    // 信息显示区域
    ImGui::BeginChild("deadlock_check_region4", { ImGui::GetContentRegionAvail().x, 0 }, true);

    ImGui::Text(u8"运行信息");
    ImGui::Separator();

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

    ImGui::EndChild();
}

bool DeadLockCheckAndRelease::detect_deadlock()
{
	deadlock_detected = false;
    auto config = ConfigManager::instance();
    int release_count = 0;
    for (int k = 0; k < pcb_queue_normal.size(); k++)
    {
		if (release_count == pcb_queue_normal.size())break;
        for (int i = 0; i < pcb_queue_normal.size(); i++)
        {
            auto& pcb = pcb_queue_normal[i];
            if (pcb->is_released_all_resource)continue;
            // 检查请求资源是否都能满足
            bool can_allocate = true;  
            for (int j = 0; j < resource_names.size(); j++)
            {
                std::string res_name = resource_names[j];
                int req_amount = pcb->resource_to_request[res_name];
                if (req_amount > config->get_resource_number(res_name))
                {
                    can_allocate = false;
                    std::string new_str_tip= std::to_string(sum_time) + u8"---进程 " + pcb->pname +
                        u8" 请求资源 " + res_name + u8" 数量: " + std::to_string(req_amount) +
						u8" 失败，系统可用数量: " + std::to_string(config->get_resource_number(res_name));
					str_tip_list.push_back(TextString(new_str_tip, TextType::Error));
                    break;
                }
                else
                {
                    std::string new_str_tip = std::to_string(sum_time) + u8"---进程 " + pcb->pname +
						u8" 请求资源 " + res_name + u8" 数量: " + std::to_string(req_amount) +
                        u8" 成功，系统可用数量: " + std::to_string(config->get_resource_number(res_name));
					str_tip_list.push_back(TextString(new_str_tip, TextType::Info));

                    config->add_resource(res_name, pcb->resource_hold[res_name]);
                }
            }
            if(can_allocate)
            {
                release_count++;
                pcb->is_released_all_resource = true;
            }
        }
    }
    deadlock_detected = (release_count != pcb_queue_normal.size());
    return deadlock_detected;
}

void DeadLockCheckAndRelease::release_all_processes()
{
    for(auto& pcb : pcb_queue_normal)
    {
        pcb->update_state(ProcessState::ActiveBlock);
        pcb->signal_and();
    }

}

void DeadLockCheckAndRelease::render_detection_info()
{
    if (deadlock_detected) {
        ImGui::Begin("死锁检测结果", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), u8"检测到死锁!");
        ImGui::Text(u8"系统中有 %d 个进程处于死锁状态", (int)pcb_queue_normal.size());
        ImGui::Separator();

        if (ImGui::Button(u8"终止所有进程解除死锁", { ImGui::GetContentRegionAvail().x, 30 })) {
            release_all_processes();
        }
        ImGui::End();
    }
}

void DeadLockCheckAndRelease::on_update_process_resource(float delta)
{
    // 进程列表显示
    const char* resource_types[] = { "Resource_A", "Resource_B", "Resource_C", "Resource_D" };
    const char* resource_request[] = { "Resource_A", "Resource_B", "Resource_C", "Resource_D" };
    // 注意：需要在PCB结构体中添加 bool show_resource_window = false;

    ImGui::BeginChild("process_list", ImVec2(0, 0), true);
    for (int i = 0; i < pcb_queue_normal.size(); i++)
    {
        auto& pcb = pcb_queue_normal[i];

        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << pcb->time_execute;

        std::string label = pcb->pname + ": " + pcb->state_name + u8" 时间:" + ss.str();

        // 使用相同样式的按钮
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.5f, 1.0f));

        if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x - 20, 30)))
        {
            pcb->show_resource_window = true;  // 显示窗口
        }
        ImGui::PopStyleColor(3);

        // 绘制进程状态指示器
        pcb->draw_state({ ImGui::GetItemRectMin().x + 10.0f, ImGui::GetItemRectMax().y - 15.0f }, 8);

        // 资源信息窗口 - 独立于按钮点击事件
        if (pcb->show_resource_window)
        {
            // 设置窗口位置和大小
            ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(200, 200), ImGuiCond_FirstUseEver);

            std::string window_title = u8"进程资源信息 - " + pcb->pname + "##" + std::to_string(i);

            if (ImGui::Begin(window_title.c_str(), &pcb->show_resource_window,
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
            {
                // 窗口美化：添加圆角和阴影
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

                // 第一部分：已分配资源
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), u8"已分配资源:");
                ImGui::Separator();

                // 为已分配资源创建表格
                ImGui::BeginTable("allocated_resources", 3,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                    ImGuiTableFlags_SizingFixedFit);

                ImGui::TableSetupColumn(u8"资源类型", ImGuiTableColumnFlags_WidthFixed, 100);
                ImGui::TableSetupColumn(u8"数量", ImGuiTableColumnFlags_WidthFixed, 60);
                ImGui::TableSetupColumn(u8"调整", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (int j = 0; j < 4; j++)
                {
                    ImGui::TableNextRow();

                    // 资源类型列
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", resource_types[j]);

                    // 数量列
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", pcb->resource_hold[resource_types[j]]);

                    // 滑块列
                    ImGui::TableSetColumnIndex(2);
                    std::string slider_label = "##alloc_" + std::to_string(i) + "_" + std::to_string(j);
                    ImGui::SetNextItemWidth(-FLT_MIN);

                    // 美化滑块
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.3f, 0.3f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.4f, 0.4f, 0.4f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));

                    ImGui::SliderInt(slider_label.c_str(), &pcb->resource_hold[resource_types[j]], 0, 3);
                    
                    //资源更新
                    if (pcb->last_hold_num[j] != pcb->resource_hold[resource_types[j]])
                    {
                        ConfigManager::instance()->sub_resource_max(resource_types[j], pcb->last_hold_num[j]);
                        ConfigManager::instance()->add_resource_max(resource_types[j], pcb->resource_hold[resource_types[j]]);
                        pcb->last_hold_num[j] = pcb->resource_hold[resource_types[j]];
                    }
                    ImGui::PopStyleColor(5);
                }
                ImGui::EndTable();

                ImGui::Spacing();
                ImGui::Spacing();

                // 第二部分：请求资源
                ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), u8"请求资源:");
                ImGui::Separator();

                // 为请求资源创建表格
                ImGui::BeginTable("requested_resources", 3,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                    ImGuiTableFlags_SizingFixedFit);

                ImGui::TableSetupColumn(u8"资源类型", ImGuiTableColumnFlags_WidthFixed, 100);
                ImGui::TableSetupColumn(u8"数量", ImGuiTableColumnFlags_WidthFixed, 60);
                ImGui::TableSetupColumn(u8"调整", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (int j = 0; j < 4; j++)
                {
                    ImGui::TableNextRow();

                    // 资源类型列
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", resource_request[j]);

                    // 数量列 - 这里显示的是当前请求的数量
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", pcb->resource_to_request[resource_request[j]]);

                    // 滑块列 - 这里调整的是最大需求
                    ImGui::TableSetColumnIndex(2);
                    std::string slider_label = "##request_" + std::to_string(i) + "_" + std::to_string(j);
                    ImGui::SetNextItemWidth(-FLT_MIN);

                    // 美化滑块
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.3f, 0.3f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.4f, 0.4f, 0.4f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.5f, 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.0f, 0.6f, 0.3f, 1.0f));

                    ImGui::SliderInt(slider_label.c_str(), &pcb->resource_to_request[resource_request[j]], 0, 3);
                    //更新最大需求量
                    pcb->resource_max[resource_request[j]] = pcb->resource_to_request[resource_request[j]]+pcb->resource_hold[resource_request[j]];

                    ImGui::PopStyleColor(5);
                }
                ImGui::EndTable();

                // 添加底部说明
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextDisabled(u8"提示：拖动滑块调整资源数量");

                ImGui::PopStyleVar(2); // 恢复样式
            }
            ImGui::End();
        }
    }
    ImGui::EndChild();
}

//recycle