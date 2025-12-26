#include "8.2deadlock_avoid.h"

void DeadLockAvoid::on_enter()
{
    str_tip_list.clear();
    process_idx = 1;
    sum_time = 0;
    current_pcb = nullptr;
    can_execute = false;
    show_safety_info = false;
    show_allocation_window = false;
    show_max_demand_window = false;
    show_request_window = false;

    // 初始化银行家算法数据结构
    resource_names = { "Resource_A", "Resource_B", "Resource_C", "Resource_D" };
    available = { 0, 0, 0, 0 };
    request_resources = { 0, 0, 0, 0 };
    allocation_edit = { 0, 0, 0, 0 };
    max_demand_edit = { 0, 0, 0, 0 };

    ConfigManager::instance()->clear_resource();
    pcb_queue_normal.clear();
    pcb_queue_can_execute.clear();

    str_tip_list.push_back(TextString(u8"死锁避免模块已启动，使用银行家算法", TextType::Info));
}

void DeadLockAvoid::on_exit()
{
    ConfigManager::instance()->clear_resource();
    pcb_queue_normal.clear();
    pcb_queue_can_execute.clear();
    str_tip_list.clear();
}

void DeadLockAvoid::on_input(const SDL_Event* event)
{
    if (ImGui::GetIO().WantCaptureKeyboard) return;
    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_e)
    {
        show_safety_info = !show_safety_info;
    }
}

void DeadLockAvoid::on_update(float delta)
{
    on_update_imgui_region1(delta);
    on_update_imgui_region2(delta);
    on_update_imgui_region3(delta);
    on_update_imgui_region4(delta);

    if (show_safety_info)
    {
        render_resource_matrices();
    }

    if (show_allocation_window)
    {
        render_process_allocation_window();
    }

    if (show_max_demand_window)
    {
        render_process_max_demand_window();
    }

    if (show_request_window)
    {
        render_resource_request_window();
    }

    if (!can_execute) return;
    sum_time += delta;
    on_update_process(delta);
}

void DeadLockAvoid::on_update_imgui_region1(float delta)
{
    // 资源管理区域 - 简化布局
    ImGui::BeginChild("deadlock_avoid_region1", { ImGui::GetContentRegionAvail().x, 80 });

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

        // 更新可用资源向量
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

        // 更新可用资源向量
        available[selected_resource_type] = std::max(0, available[selected_resource_type] - to_remove);

        std::string tip = std::to_string(sum_time) + u8"---删除资源: " + resource_name +
            u8" 数量: " + std::to_string(to_remove);
        str_tip_list.push_back(TextString(tip, TextType::Info));
    }

    ImGui::EndChild();
    ImGui::Separator();
}

void DeadLockAvoid::on_update_imgui_region2(float delta)
{
    // 进程管理区域
    ImGui::BeginChild("deadlock_avoid_region2", { ImGui::GetContentRegionAvail().x * 0.6f, 300 });

    ImGui::Text(u8"进程管理");
    ImGui::Separator();

    // 创建进程按钮
    if (ImGui::Button(u8"创建新进程", { ImGui::GetContentRegionAvail().x, 30 }))
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
            pcb->time_execute = 2.0;
            pcb->pname = u8"Process" + std::to_string(process_idx++);
            pcb->update_state(ProcessState::ActiveBlock);

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
                    release_resources_from_process(std::distance(pcb_queue_normal.begin(),
                        std::find(pcb_queue_normal.begin(), pcb_queue_normal.end(), sp)));
                });

            std::string tip = std::to_string(sum_time) + u8"---进程创建:  " + pcb->pname;
            str_tip_list.push_back(TextString(tip, TextType::Info));
            pcb_queue_normal.push_back(pcb);

            // 初始化银行家算法矩阵
            int new_index = pcb_queue_normal.size() - 1;
            if (new_index >= max_demand.size())
            {
                max_demand.push_back({ 0, 0, 0, 0 });
                allocation.push_back({ 0, 0, 0, 0 });
                need.push_back({ 0, 0, 0, 0 });
            }
        }
    }

    // 进程列表显示
    ImGui::BeginChild("process_list", { 0, 0 }, true);
    for (int i = 0; i < pcb_queue_normal.size(); i++)
    {
        auto& pcb = pcb_queue_normal[i];

        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << pcb->time_execute;

        std::string label = pcb->pname + ": " + pcb->state_name + u8" 时间:" + ss.str();

        if (ImGui::Button(label.c_str(), { ImGui::GetContentRegionAvail().x * 0.7f, 30 }))
        {
            current_pcb = pcb.get();
            selected_process = i;
        }

        ImGui::SameLine();

        if (ImGui::Button((u8"设置##" + std::to_string(i)).c_str(), { ImGui::GetContentRegionAvail().x, 30 }))
        {
            selected_process = i;
            current_pcb = pcb.get();
            show_allocation_window = true;
            // 复制当前分配值到编辑缓冲区
            if (i < allocation.size())
            {
                allocation_edit = allocation[i];
                max_demand_edit = max_demand[i];
            }
        }
    }
    ImGui::EndChild();

    ImGui::EndChild();
    ImGui::SameLine();

    // 资源状态区域
    ImGui::BeginChild("resource_status", { 0, 300 }, true);
    ImGui::Text(u8"资源状态");
    ImGui::Separator();

    const auto& resource_pool = ConfigManager::instance()->get_resource_pool();
    for (int i = 0; i < resource_names.size(); i++)
    {
        const auto& resource_name = resource_names[i];
        int current_amount = ConfigManager::instance()->get_resource_number(resource_name);
        int max_amount = ConfigManager::instance()->get_resource_number_max(resource_name);

        ImGui::Image(ResourcesManager::instance()->find_texture("Resource"), { 20,20 });
        ImGui::SameLine();
        ImGui::Text(u8"%s: %d/%d", resource_name.c_str(), current_amount, max_amount);
        ImGui::SameLine();
        ImGui::Text(u8"可用: %d", available[i]);

        float progress = max_amount > 0 ? (float)current_amount / max_amount : 0.0f;
        ImGui::ProgressBar(progress, { ImGui::GetContentRegionAvail().x, 20 });
    }

    ImGui::EndChild();
    ImGui::Separator();
}

void DeadLockAvoid::on_update_imgui_region3(float delta)
{
    // 银行家算法控制区域
    ImGui::BeginChild("deadlock_avoid_region3", { ImGui::GetContentRegionAvail().x, 60 });

    ImGui::Text(u8"银行家算法控制");
    ImGui::Separator();

    if (ImGui::Button(u8"开始执行", { 100, 30 }))
    {
        if (!can_execute)
        {
            str_tip_list.push_back(TextString(u8"开始执行银行家算法", TextType::Info));
            // 初始安全性检查
            if (banker_algorithm_safety_check())
            {
                str_tip_list.push_back(TextString(u8"系统处于安全状态，可以开始执行", TextType::Info));
            }
            else
            {
                str_tip_list.push_back(TextString(u8"警告：系统处于不安全状态！", TextType::Warning));
            }
        }
        can_execute = true;
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"安全性检查", { 100, 30 }))
    {
        if (banker_algorithm_safety_check())
        {
            std::string tip = std::to_string(sum_time) + u8"---系统处于安全状态";
            str_tip_list.push_back(TextString(tip, TextType::Info));
        }
        else
        {
            std::string tip = std::to_string(sum_time) + u8"---警告：系统处于不安全状态！";
            str_tip_list.push_back(TextString(tip, TextType::Warning));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"资源请求", { 100, 30 }))
    {
        show_request_window = true;
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"显示矩阵", { 100, 30 }))
    {
        show_safety_info = !show_safety_info;
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"清空日志", { 100, 30 }))
    {
        str_tip_list.clear();
    }

    ImGui::EndChild();
    ImGui::Separator();
}

void DeadLockAvoid::on_update_imgui_region4(float delta)
{
    // 信息显示区域
    ImGui::BeginChild("deadlock_avoid_region4", { ImGui::GetContentRegionAvail().x, 0 }, true);

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

void DeadLockAvoid::on_update_process(float delta)
{
    // 注释掉的代码保持不变，但其中的str_tip也需要修改，因为这是注释代码，所以不需要修改
    //// 首先检查并移除已完成但仍在队列中的进程
    //for (auto it = pcb_queue_can_execute.begin(); it != pcb_queue_can_execute.end(); )
    //{
    //    if ((*it)->current_state == ProcessState::Finish)
    //    {
    //        it = pcb_queue_can_execute.erase(it);
    //    }
    //    else
    //    {
    //        ++it;
    //    }
    //}
}

bool DeadLockAvoid::banker_algorithm_safety_check()
{
    int process_count = pcb_queue_normal.size();
    int resource_count = resource_names.size();

    std::vector<int> work = available;
    std::vector<bool> finish(process_count, false);
    safe_sequence.clear();

    bool found;
    do {
        found = false;
        for (int i = 0; i < process_count; i++)
        {
            if (!finish[i])
            {
                bool can_allocate = true;
                for (int j = 0; j < resource_count; j++)
                {
                    if (need[i][j] > work[j])
                    {
                        can_allocate = false;
                        break;
                    }
                }

                if (can_allocate)
                {
                    // 模拟分配资源
                    for (int j = 0; j < resource_count; j++)
                    {
                        work[j] += allocation[i][j];
                    }
                    finish[i] = true;
                    safe_sequence.push_back(i);
                    found = true;
                }
            }
        }
    } while (found);

    // 检查是否所有进程都完成
    for (int i = 0; i < process_count; i++)
    {
        if (!finish[i])
        {
            return false; // 不安全状态
        }
    }

    return true; // 安全状态
}

bool DeadLockAvoid::banker_algorithm_resource_request(int process_index, const std::vector<int>& request)
{
    int resource_count = resource_names.size();

    // 检查请求是否超过需求
    for (int i = 0; i < resource_count; i++)
    {
        if (request[i] > need[process_index][i])
        {
            str_tip_list.push_back(TextString(u8"错误：请求超过最大需求", TextType::Error));
            return false;
        }
    }

    // 检查请求是否超过可用资源
    for (int i = 0; i < resource_count; i++)
    {
        if (request[i] > available[i])
        {
            str_tip_list.push_back(TextString(u8"错误：请求超过可用资源", TextType::Error));
            return false;
        }
    }

    // 尝试分配资源
    for (int i = 0; i < resource_count; i++)
    {
        available[i] -= request[i];
        allocation[process_index][i] += request[i];
        need[process_index][i] -= request[i];
    }

    // 检查分配后是否安全
    bool safe = banker_algorithm_safety_check();

    // 如果不安全，回滚分配
    if (!safe)
    {
        for (int i = 0; i < resource_count; i++)
        {
            available[i] += request[i];
            allocation[process_index][i] -= request[i];
            need[process_index][i] += request[i];
        }
    }

    return safe;
}

void DeadLockAvoid::allocate_resources_to_process(int process_index)
{
    if (process_index < 0 || process_index >= pcb_queue_normal.size())
        return;

    auto& pcb = pcb_queue_normal[process_index];

    bool resources_allocated = false;
    for (int i = 0; i < resource_names.size(); i++)
    {
        if (allocation[process_index][i] > 0)
        {
            // 从系统资源中扣除
            ConfigManager::instance()->sub_resource(resource_names[i], allocation[process_index][i]);
            // 添加到进程持有资源
            pcb->resource_hold[resource_names[i]] += allocation[process_index][i];

            std::string tip = std::to_string(sum_time) + u8"---分配资源: " + resource_names[i] +
                u8" 数量: " + std::to_string(allocation[process_index][i]) + u8" 给进程 " + pcb->pname;
            str_tip_list.push_back(TextString(tip, TextType::Info));

            resources_allocated = true;
        }
    }
}

void DeadLockAvoid::release_resources_from_process(int process_index)
{
    if (process_index < 0 || process_index >= pcb_queue_normal.size())
        return;

    auto& pcb = pcb_queue_normal[process_index];

    bool resources_released = false;
    for (int i = 0; i < resource_names.size(); i++)
    {
        if (allocation[process_index][i] > 0)
        {
            // 将资源返还给系统
            ConfigManager::instance()->add_resource(resource_names[i], allocation[process_index][i]);
            // 更新银行家算法数据结构
            available[i] += allocation[process_index][i];

            std::string tip = std::to_string(sum_time) + u8"---释放资源: " + resource_names[i] +
                u8" 数量: " + std::to_string(allocation[process_index][i]);
            str_tip_list.push_back(TextString(tip, TextType::Info));

            allocation[process_index][i] = 0;
            max_demand[process_index][i] = 0;
            need[process_index][i] = 0;
            // 清空进程持有的资源
            pcb->resource_hold[resource_names[i]] = 0;

            resources_released = true;
        }
    }

    if (resources_released)
    {
        std::string tip = std::to_string(sum_time) + u8"---进程 " + pcb->pname + u8" 释放所有资源";
        str_tip_list.push_back(TextString(tip, TextType::Info));
    }
}

void DeadLockAvoid::update_process_allocation(int process_index)
{
    // 更新分配矩阵
    allocation[process_index] = allocation_edit;

    // 更新需求矩阵 Need = Max - Allocation
    for (int i = 0; i < resource_names.size(); i++)
    {
        need[process_index][i] = max_demand[process_index][i] - allocation[process_index][i];
        if (need[process_index][i] < 0)
        {
            need[process_index][i] = 0;
        }
    }

    std::string tip = std::to_string(sum_time) + u8"---更新进程 " + pcb_queue_normal[process_index]->pname +
        u8" 的资源分配";
    str_tip_list.push_back(TextString(tip, TextType::Info));
}

void DeadLockAvoid::update_process_max_demand(int process_index)
{
    // 更新最大需求矩阵
    max_demand[process_index] = max_demand_edit;

    // 更新需求矩阵 Need = Max - Allocation
    for (int i = 0; i < resource_names.size(); i++)
    {
        need[process_index][i] = max_demand[process_index][i] - allocation[process_index][i];
        if (need[process_index][i] < 0)
        {
            need[process_index][i] = 0;
        }
    }

    std::string tip = std::to_string(sum_time) + u8"---更新进程 " + pcb_queue_normal[process_index]->pname +
        u8" 的最大需求";
    str_tip_list.push_back(TextString(tip, TextType::Info));
}

void DeadLockAvoid::render_resource_matrices()
{
    ImGui::SetNextWindowSize({ 800, 500 }, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(u8"银行家算法资源矩阵", &show_safety_info))
    {
        if (ImGui::BeginTabBar("ResourceMatrices"))
        {
            // 可用资源向量
            if (ImGui::BeginTabItem(u8"可用资源"))
            {
                ImGui::Text(u8"可用资源向量 (Available):");
                ImGui::Separator();

                for (int i = 0; i < resource_names.size(); i++)
                {
                    ImGui::Text(u8"%s: %d", resource_names[i].c_str(), available[i]);
                }

                ImGui::EndTabItem();
            }

            // 最大需求矩阵
            if (ImGui::BeginTabItem(u8"最大需求"))
            {
                ImGui::Text(u8"最大需求矩阵 (Max):");
                ImGui::Separator();

                // 表头
                ImGui::Text(u8"进程");
                for (int j = 0; j < resource_names.size(); j++)
                {
                    ImGui::SameLine();
                    ImGui::Text(u8"%s", resource_names[j].c_str());
                }

                ImGui::Separator();

                // 数据行
                for (int i = 0; i < pcb_queue_normal.size(); i++)
                {
                    ImGui::Text(u8"%s", pcb_queue_normal[i]->pname.c_str());
                    for (int j = 0; j < resource_names.size(); j++)
                    {
                        ImGui::SameLine();
                        ImGui::Text(u8"%d", max_demand[i][j]);
                    }
                }

                ImGui::EndTabItem();
            }

            // 分配矩阵
            if (ImGui::BeginTabItem(u8"已分配"))
            {
                ImGui::Text(u8"分配矩阵 (Allocation):");
                ImGui::Separator();

                // 表头
                ImGui::Text(u8"进程");
                for (int j = 0; j < resource_names.size(); j++)
                {
                    ImGui::SameLine();
                    ImGui::Text(u8"%s", resource_names[j].c_str());
                }

                ImGui::Separator();

                // 数据行
                for (int i = 0; i < pcb_queue_normal.size(); i++)
                {
                    ImGui::Text(u8"%s", pcb_queue_normal[i]->pname.c_str());
                    for (int j = 0; j < resource_names.size(); j++)
                    {
                        ImGui::SameLine();
                        ImGui::Text(u8"%d", allocation[i][j]);
                    }
                }

                ImGui::EndTabItem();
            }

            // 需求矩阵
            if (ImGui::BeginTabItem(u8"需求矩阵"))
            {
                ImGui::Text(u8"需求矩阵 (Need = Max - Allocation):");
                ImGui::Separator();

                // 表头
                ImGui::Text(u8"进程");
                for (int j = 0; j < resource_names.size(); j++)
                {
                    ImGui::SameLine();
                    ImGui::Text(u8"%s", resource_names[j].c_str());
                }

                ImGui::Separator();

                // 数据行
                for (int i = 0; i < pcb_queue_normal.size(); i++)
                {
                    ImGui::Text(u8"%s", pcb_queue_normal[i]->pname.c_str());
                    for (int j = 0; j < resource_names.size(); j++)
                    {
                        ImGui::SameLine();
                        ImGui::Text(u8"%d", need[i][j]);
                    }
                }

                ImGui::EndTabItem();
            }

            // 安全序列
            if (ImGui::BeginTabItem(u8"安全序列"))
            {
                ImGui::Text(u8"安全序列:");
                ImGui::Separator();

                if (safe_sequence.empty())
                {
                    ImGui::Text(u8"当前没有安全序列");
                }
                else
                {
                    std::string sequence_str;
                    for (int i = 0; i < safe_sequence.size(); i++)
                    {
                        if (i > 0) sequence_str += u8" -> ";
                        sequence_str += pcb_queue_normal[safe_sequence[i]]->pname;
                    }
                    ImGui::Text(u8"%s", sequence_str.c_str());
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void DeadLockAvoid::render_process_allocation_window()
{
    ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(u8"设置进程资源分配", &show_allocation_window))
    {
        if (selected_process < pcb_queue_normal.size())
        {
            auto& pcb = pcb_queue_normal[selected_process];
            ImGui::Text(u8"进程: %s", pcb->pname.c_str());
            ImGui::Separator();

            ImGui::Text(u8"设置已分配资源:");

            // 显示表格
            ImGui::Columns(4, "allocation_columns");
            ImGui::Text(u8"资源"); ImGui::NextColumn();
            ImGui::Text(u8"最大需求"); ImGui::NextColumn();
            ImGui::Text(u8"已分配"); ImGui::NextColumn();
            ImGui::Text(u8"需求"); ImGui::NextColumn();
            ImGui::Separator();

            for (int i = 0; i < resource_names.size(); i++)
            {
                ImGui::Text(u8"%s", resource_names[i].c_str()); ImGui::NextColumn();
                ImGui::Text(u8"%d", max_demand[selected_process][i]); ImGui::NextColumn();

                ImGui::SetNextItemWidth(80);
                ImGui::InputInt(("##alloc_" + std::to_string(i)).c_str(), &allocation_edit[i]);
                ImGui::NextColumn();

                // 计算并显示需求
                int current_need = max_demand[selected_process][i] - allocation_edit[i];
                ImGui::Text(u8"%d", current_need); ImGui::NextColumn();
            }
            ImGui::Columns(1);

            ImGui::Separator();

            if (ImGui::Button(u8"设置最大需求", { ImGui::GetContentRegionAvail().x / 2, 30 }))
            {
                show_allocation_window = false;
                show_max_demand_window = true;
            }

            ImGui::SameLine();

            if (ImGui::Button(u8"应用分配", { ImGui::GetContentRegionAvail().x, 30 }))
            {
                // 检查分配是否有效
                bool valid = true;
                for (int i = 0; i < resource_names.size(); i++)
                {
                    if (allocation_edit[i] > max_demand[selected_process][i])
                    {
                        str_tip_list.push_back(TextString(u8"错误：分配数量不能超过最大需求", TextType::Error));
                        valid = false;
                        break;
                    }
                }

                if (valid)
                {
                    update_process_allocation(selected_process);
                    show_allocation_window = false;
                }
            }
        }
        else
        {
            ImGui::Text(u8"无效的进程选择");
        }
    }
    ImGui::End();
}

void DeadLockAvoid::render_process_max_demand_window()
{
    ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(u8"设置进程最大需求", &show_max_demand_window))
    {
        if (selected_process < pcb_queue_normal.size())
        {
            auto& pcb = pcb_queue_normal[selected_process];
            ImGui::Text(u8"进程: %s", pcb->pname.c_str());
            ImGui::Separator();

            ImGui::Text(u8"设置最大需求:");

            // 显示表格
            ImGui::Columns(4, "max_demand_columns");
            ImGui::Text(u8"资源"); ImGui::NextColumn();
            ImGui::Text(u8"最大需求"); ImGui::NextColumn();
            ImGui::Text(u8"已分配"); ImGui::NextColumn();
            ImGui::Text(u8"需求"); ImGui::NextColumn();
            ImGui::Separator();

            for (int i = 0; i < resource_names.size(); i++)
            {
                ImGui::Text(u8"%s", resource_names[i].c_str()); ImGui::NextColumn();

                ImGui::SetNextItemWidth(80);
                ImGui::InputInt(("##max_" + std::to_string(i)).c_str(), &max_demand_edit[i]);
                ImGui::NextColumn();

                ImGui::Text(u8"%d", allocation[selected_process][i]); ImGui::NextColumn();

                // 计算并显示需求
                int current_need = max_demand_edit[i] - allocation[selected_process][i];
                ImGui::Text(u8"%d", current_need); ImGui::NextColumn();
            }
            ImGui::Columns(1);

            ImGui::Separator();

            if (ImGui::Button(u8"返回分配设置", { ImGui::GetContentRegionAvail().x / 2, 30 }))
            {
                show_max_demand_window = false;
                show_allocation_window = true;
            }

            ImGui::SameLine();

            if (ImGui::Button(u8"应用最大需求", { ImGui::GetContentRegionAvail().x, 30 }))
            {
                // 检查最大需求是否有效
                bool valid = true;
                for (int i = 0; i < resource_names.size(); i++)
                {
                    if (max_demand_edit[i] < allocation[selected_process][i])
                    {
                        str_tip_list.push_back(TextString(u8"错误：最大需求不能小于已分配数量", TextType::Error));
                        valid = false;
                        break;
                    }
                }

                if (valid)
                {
                    update_process_max_demand(selected_process);
                    show_max_demand_window = false;
                    show_allocation_window = true;
                }
            }
        }
        else
        {
            ImGui::Text(u8"无效的进程选择");
        }
    }
    ImGui::End();
}

void DeadLockAvoid::render_resource_request_window()
{
    ImGui::SetNextWindowSize({ 400, 300 }, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(u8"资源请求", &show_request_window))
    {
        if (pcb_queue_normal.empty())
        {
            ImGui::Text(u8"没有可用的进程");
        }
        else
        {
            // 进程选择
            ImGui::Text(u8"选择进程:");
            std::vector<const char*> process_items;
            for (int i = 0; i < pcb_queue_normal.size(); i++)
            {
                process_items.push_back(pcb_queue_normal[i]->pname.c_str());
            }

            ImGui::SetNextItemWidth(200);
            ImGui::Combo("##process_selection", &selected_process, process_items.data(), process_items.size());

            ImGui::Separator();
            ImGui::Text(u8"资源请求量:");

            // 资源请求输入
            for (int i = 0; i < resource_names.size(); i++)
            {
                ImGui::Text(u8"%s:", resource_names[i].c_str());
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100);
                ImGui::InputInt(("##request_" + std::to_string(i)).c_str(), &request_resources[i]);
                ImGui::SameLine();
                ImGui::Text(u8"可用: %d", available[i]);
            }

            ImGui::Separator();

            if (ImGui::Button(u8"发送请求", { ImGui::GetContentRegionAvail().x, 30 }))
            {
                if (banker_algorithm_resource_request(selected_process, request_resources))
                {
                    std::string tip = std::to_string(sum_time) + u8"---请求批准: 进程 " +
                        pcb_queue_normal[selected_process]->pname + u8" 获得资源";
                    str_tip_list.push_back(TextString(tip, TextType::Info));
                    allocate_resources_to_process(selected_process);
                }
                else
                {
                    std::string tip = std::to_string(sum_time) + u8"---请求拒绝: 进程 " +
                        pcb_queue_normal[selected_process]->pname + u8" 的资源请求会导致系统不安全";
                    str_tip_list.push_back(TextString(tip, TextType::Warning));
                }
                show_request_window = false;
            }
        }
    }
    ImGui::End();
}
//finish