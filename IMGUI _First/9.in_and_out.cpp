#include "9.in_and_out.h"

// on_enter，进入当前样例的时刻调用，用于初始化样例数据
void InAndOut::on_enter()
{
    // 重置所有变量
    algorithm_choice = 0;
    virtual_memory_size = 16;
    physical_memory_size = 4;
    sequence_length = 20;
    current_time = 0;
    access_index = 0;

    simulation_speed = 1.0f;
    elapsed_time = 0.0f;
    is_auto_mode = false;
    is_running = false;
    is_finished = false;
    show_help = false;

    // 清除所有数据结构
    pages.clear();
    access_sequence.clear();
    memory_frames.clear();
    memory_set.clear();
    logs.clear();
    last_replaced.clear();

    // 初始化页面
    for (int i = 0; i < virtual_memory_size; i++) {
        pages.push_back(Page(i));
    }

    // 生成初始访问序列
    generate_access_sequence();

    // 重置统计信息
    reset_statistics();

    // 初始状态
    current_status = u8"就绪 - 点击开始按钮开始模拟";
    last_operation = u8"等待开始...";

    logs.push_back(u8"系统初始化完成");
    logs.push_back(u8"虚拟内存大小: " + std::to_string(virtual_memory_size) + u8" 页");
    logs.push_back(u8"物理内存大小: " + std::to_string(physical_memory_size) + u8" 页");
    logs.push_back(u8"访问序列长度: " + std::to_string(sequence_length));
}

// on_exit，离开当前样例的时刻调用，用于清理样例数据，释放内存
void InAndOut::on_exit()
{
    ConfigManager::instance()->clear_resource();
    logs.clear();
}

// on_update，每帧调用，用于更新样例逻辑和渲染ImGui界面
void InAndOut::on_update(float delta)
{
    on_update_imgui_region1(delta);
    on_update_imgui_region2(delta);
    on_update_imgui_region3(delta);

    if (is_running && !is_finished) {
        elapsed_time += delta * simulation_speed;
        if (elapsed_time >= 0.5f) {  // 每0.5秒执行一次访问
            elapsed_time = 0.0f;
            if (is_auto_mode) {
                next_access();
            }
        }
    }
}

// 区域1：控制面板
void InAndOut::on_update_imgui_region1(float delta)
{
    ImGui::BeginChild(u8"控制面板", { ImGui::GetContentRegionAvail().x, 180 });

    ImGui::Text(u8"算法设置"); ImGui::Separator();

    // 算法选择
    ImGui::RadioButton(u8"LRU（最近最久未使用）", &algorithm_choice, 0);
    ImGui::SameLine();
    ImGui::RadioButton(u8"LFU（最少使用）", &algorithm_choice, 1);

    // 内存参数设置
    ImGui::Spacing();
    ImGui::Text(u8"内存设置");
    ImGui::Columns(2, u8"内存设置列", false);

    ImGui::Text(u8"虚拟内存大小（页）:");
    ImGui::SetNextItemWidth(150);
    if (ImGui::SliderInt(u8"##虚拟内存", &virtual_memory_size, 8, 64, "%d")) {
        if (virtual_memory_size < physical_memory_size * 2) {
            physical_memory_size = virtual_memory_size / 2;
        }
    }

    ImGui::NextColumn();

    ImGui::Text(u8"物理内存大小（页）:");
    ImGui::SetNextItemWidth(150);
    ImGui::SliderInt(u8"##物理内存", &physical_memory_size, 2, 16, "%d");

    ImGui::Columns(1);

    ImGui::Text(u8"访问序列长度:");
    ImGui::SetNextItemWidth(150);
    ImGui::SliderInt(u8"##序列长度", &sequence_length, 10, 100, "%d");

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    // 控制按钮
    ImGui::Columns(3, u8"控制按钮列", false);

    if (ImGui::Button(u8"生成新序列", { 150, 30 })) {
        generate_access_sequence();
        logs.push_back(u8"已生成新的访问序列");
    }

    ImGui::NextColumn();

    if (ImGui::Button(is_running ? u8"暂停" : u8"开始", { 150, 30 })) {
        is_running = !is_running;
        if (is_running) {
            current_status = u8"运行中...";
            logs.push_back(u8"模拟开始运行");
        }
        else {
            current_status = u8"已暂停";
            logs.push_back(u8"模拟暂停");
        }
    }

    ImGui::NextColumn();

    if (ImGui::Button(u8"结束并统计", { 150, 30 })) {
        end_simulation();
    }

    ImGui::Columns(1);

    // 模拟速度控制
    ImGui::Spacing();
    ImGui::Text(u8"模拟速度:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    ImGui::SliderFloat(u8"##速度", &simulation_speed, 0.1f, 5.0f, u8"x%.1f");

    // 自动模式开关
    ImGui::SameLine();
    ImGui::Checkbox(u8"自动模式", &is_auto_mode);

    // 单步执行按钮（非自动模式下）
    if (!is_auto_mode && is_running && !is_finished) {
        ImGui::SameLine();
        if (ImGui::Button(u8"下一步", { 80, 30 })) {
            next_access();
        }
    }

    ImGui::EndChild();
    ImGui::Separator();
}

// 区域2：内存状态显示
void InAndOut::on_update_imgui_region2(float delta)
{
    ImGui::BeginChild(u8"显示区域", { ImGui::GetContentRegionAvail().x * 0.6f, 0 });

    ImGui::Text(u8"内存状态"); ImGui::Separator();

    // 当前状态显示
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), u8"当前状态: %s", current_status.c_str());
    ImGui::Text(u8"最后操作: %s", last_operation.c_str());

    // 显示算法名称
    std::string algo_name = (algorithm_choice == 0) ? u8"LRU" : u8"LFU";
    ImGui::Text(u8"当前算法: %s", algo_name.c_str());

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    // 内存表显示
    draw_memory_table();

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    // 访问序列显示
    draw_page_list();

    ImGui::EndChild();
    ImGui::SameLine();
}
        
// 区域3：统计信息和日志
void InAndOut::on_update_imgui_region3(float delta)
{
    ImGui::BeginChild(u8"统计区域", { 0, 0 }, true);

    ImGui::Text(u8"统计信息"); ImGui::Separator();

    // 绘制统计信息
    draw_statistics();

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    ImGui::Text(u8"运行日志"); ImGui::Separator();

    // 日志显示区域
    ImGui::BeginChild(u8"日志区域", { 0, -30 }, true);

    for (const auto& log : logs) {
        ImGui::TextUnformatted(log.c_str());
    }

    // 自动滚动到底部
    static size_t last_log_count = 0;
    if (last_log_count != logs.size()) {
        ImGui::SetScrollHereY(1.0f);
        last_log_count = logs.size();
    }

    ImGui::EndChild();

    // 清空日志按钮
    if (ImGui::Button(u8"清空日志", { ImGui::GetContentRegionAvail().x, 25 })) {
        if (logs.size() > 10) {
            // 保留最近的10条日志
            std::vector<std::string> recent_logs;
            size_t start = logs.size() > 10 ? logs.size() - 10 : 0;
            for (size_t i = start; i < logs.size(); i++) {
                recent_logs.push_back(logs[i]);
            }
            logs = recent_logs;
        }
    }

    ImGui::EndChild();
}

// 生成随机访问序列
void InAndOut::generate_access_sequence()
{
    access_sequence.clear();
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 生成随机访问序列，有一定局部性
    int current = std::rand() % virtual_memory_size;
    for (int i = 0; i < sequence_length; i++) {
        // 80%的概率访问附近页面，20%的概率随机访问
        if (std::rand() % 100 < 80 && virtual_memory_size > 4) {
            // 访问当前页面附近的页面
            int offset = (std::rand() % 7) - 3; // -3到+3的范围
            current = (current + offset + virtual_memory_size) % virtual_memory_size;
        }
        else {
            // 完全随机访问
            current = std::rand() % virtual_memory_size;
        }
        access_sequence.push_back(current);
    }

    // 重置访问索引
    access_index = 0;
    current_time = 0;
    is_finished = false;

    // 清空内存
    memory_frames.clear();
    memory_set.clear();
    last_replaced.clear();

    // 重置页面状态
    for (auto& page : pages) {
        page.in_memory = false;
        page.last_access = 0;
        page.access_count = 0;
    }

    // 重置统计
    reset_statistics();

    logs.push_back(u8"已生成新的访问序列，长度: " + std::to_string(sequence_length));
    logs.push_back(u8"序列: " + std::to_string(access_sequence[0]) + u8" ...");
}

// 重置统计信息
void InAndOut::reset_statistics()
{
    hit_count = 0;
    fault_count = 0;
    total_access = 0;
    hit_rate = 0.0f;
    fault_rate = 0.0f;
}

// 更新统计信息
void InAndOut::update_statistics(bool hit)
{
    total_access++;
    
    if (total_access > sequence_length)
    {
		total_access = sequence_length;
        return;
    }
    if (hit) {
        hit_count++;
    }
    else {
        fault_count++;
    }
    total_access = std::min(total_access, sequence_length);

    if (total_access > 0) {
        hit_rate = static_cast<float>(hit_count) / total_access * 100.0f;
        fault_rate = static_cast<float>(fault_count) / total_access * 100.0f;
    }
}

// 执行下一次页面访问
void InAndOut::next_access()
{
    if (access_index >= sequence_length || is_finished) {
        is_running = false;
        is_finished = true;
        current_status = u8"模拟结束";
        logs.push_back(u8"模拟结束");
        return;
    }

    int page_id = access_sequence[access_index];
    access_index++;
    current_time++;

    // 执行页面访问
    access_page(page_id);

    // 检查是否结束
    if (access_index >= sequence_length) {
        end_simulation();
    }
}

// 页面访问
void InAndOut::access_page(int page_id)
{
    // 检查页面是否在内存中
    bool hit = (memory_set.find(page_id) != memory_set.end());

    if (hit) {
        // 页面命中
        pages[page_id].update(current_time);

        // 如果是LRU算法，需要更新内存中的顺序
        if (algorithm_choice == 0) {
            memory_frames.remove(page_id);
            memory_frames.push_front(page_id);
        }

        current_status = u8"页面命中";
        last_operation = u8"访问页面 " + std::to_string(page_id) + u8" - 命中";
        logs.push_back(u8"时间 " + std::to_string(current_time) +
            u8": 访问页面 " + std::to_string(page_id) + u8" - 命中");
    }
    else {
        // 页面缺页
        handle_page_fault(page_id);
        current_status = u8"页面缺页";
        last_operation = u8"访问页面 " + std::to_string(page_id) + u8" - 缺页";
        logs.push_back(u8"时间 " + std::to_string(current_time) +
            u8": 访问页面 " + std::to_string(page_id) + u8" - 缺页");
    }

    update_statistics(hit);
}

// 处理缺页
void InAndOut::handle_page_fault(int page_id)
{
    // 更新页面信息
    pages[page_id].update(current_time);

    if (memory_frames.size() < physical_memory_size) {
        // 物理内存还有空间，直接装入
        memory_frames.push_front(page_id);
        memory_set.insert(page_id);
        pages[page_id].in_memory = true;
        logs.push_back(u8"物理内存还有空间，页面 " + std::to_string(page_id) + u8" 直接装入");
    }
    else {
        // 需要页面置换
        if (algorithm_choice == 0) {
            replace_page_lru();
        }
        else {
            replace_page_lfu();
        }

        // 将新页面装入
        memory_frames.push_front(page_id);
        memory_set.insert(page_id);
        pages[page_id].in_memory = true;
    }
}

// LRU页面置换
void InAndOut::replace_page_lru()
{
    // LRU: 移除最近最久未使用的页面（列表末尾）
    int page_to_remove = memory_frames.back();
    memory_frames.pop_back();
    memory_set.erase(page_to_remove);
    pages[page_to_remove].in_memory = false;
    last_replaced.push_back(page_to_remove);

    logs.push_back(u8"LRU算法: 替换页面 " + std::to_string(page_to_remove) +
        u8" (最近最久未使用)");
}

// LFU页面置换
void InAndOut::replace_page_lfu()
{
    // LFU: 找到访问次数最少的页面
    int min_access = INT_MAX;
    int page_to_remove = -1;

    for (int page_id : memory_frames) {
        if (pages[page_id].access_count < min_access) {
            min_access = pages[page_id].access_count;
            page_to_remove = page_id;
        }
    }

    if (page_to_remove != -1) {
        memory_frames.remove(page_to_remove);
        memory_set.erase(page_to_remove);
        pages[page_to_remove].in_memory = false;
        last_replaced.push_back(page_to_remove);

        logs.push_back(u8"LFU算法: 替换页面 " + std::to_string(page_to_remove) +
            u8" (访问次数: " + std::to_string(min_access) + u8")");
    }
}

// 结束模拟并显示统计信息
void InAndOut::end_simulation()
{
    is_running = false;
    is_finished = true;
    current_status = u8"模拟结束";

    // 计算最终统计
    update_statistics(false); // 确保统计信息是最新的

    logs.push_back(u8"================== 模拟结束 ==================");
    logs.push_back(u8"命中次数: " + std::to_string(hit_count));
    logs.push_back(u8"缺页次数: " + std::to_string(fault_count));
    logs.push_back(u8"总访问次数: " + std::to_string(total_access));
    logs.push_back(u8"命中率: " + std::to_string(hit_rate) + u8"%");
    logs.push_back(u8"缺页率: " + std::to_string(fault_rate) + u8"%");
    logs.push_back(u8"============================================");
}

// 绘制内存表
void InAndOut::draw_memory_table()
{
    ImGui::Text(u8"物理内存状态:");

    // 创建表格
    if (ImGui::BeginTable(u8"内存表", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // 表头
        ImGui::TableSetupColumn(u8"帧号");
        ImGui::TableSetupColumn(u8"页面号");
        ImGui::TableSetupColumn(u8"最近访问时间");
        ImGui::TableSetupColumn(u8"访问次数");
        ImGui::TableSetupColumn(u8"状态");
        ImGui::TableHeadersRow();

        // 显示物理内存中的页面
        int frame_index = 0;
        for (auto it = memory_frames.begin(); it != memory_frames.end(); ++it, ++frame_index) {
            int page_id = *it;

            ImGui::TableNextRow();

            // 帧号
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", frame_index);

            // 页面号
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", page_id);

            // 最近访问时间
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", pages[page_id].last_access);

            // 访问次数
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", pages[page_id].access_count);

            // 状态
            ImGui::TableSetColumnIndex(4);
            if (last_replaced.size() > 0 && page_id == last_replaced.back()) {
                // 如果是最近被替换的页面，显示特殊标记
                ImGui::TextColored(fault_color, u8"新装入");
            }
            else {
                ImGui::TextColored(memory_color, u8"在内存中");
            }
        }

        // 显示空闲帧
        for (int i = memory_frames.size(); i < physical_memory_size; i++) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", i);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text(u8"-");

            ImGui::TableSetColumnIndex(2);
            ImGui::Text(u8"-");

            ImGui::TableSetColumnIndex(3);
            ImGui::Text(u8"-");

            ImGui::TableSetColumnIndex(4);
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), u8"空闲");
        }

        ImGui::EndTable();
    }

    // 显示最近被替换的页面信息
    if (!last_replaced.empty()) {
        ImGui::Spacing();
        ImGui::Text(u8"最近被替换的页面: ");
        for (int i = 0; i < std::min(3, (int)last_replaced.size()); i++) {
            ImGui::SameLine();
            ImGui::TextColored(fault_color, "%d", last_replaced[last_replaced.size() - 1 - i]);
            if (i < 2 && i < (int)last_replaced.size() - 1) {
                ImGui::SameLine();
                ImGui::Text(u8",");
            }
        }
    }
}

// 绘制页面列表
void InAndOut::draw_page_list()
{
    ImGui::Text(u8"访问序列 (当前: %d/%d):", access_index, sequence_length);

    // 显示访问序列，当前访问的页面高亮显示
    ImGui::BeginChild(u8"序列区域", { 0, 100 }, true);

    int cols = 9;
    int rows = (sequence_length + cols - 1) / cols;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int idx = row * cols + col;
            if (idx >= sequence_length) break;

            if (col > 0) ImGui::SameLine();

            // 设置按钮宽度
            ImGui::PushItemWidth(35);

            int page_id = access_sequence[idx];
            std::string label = std::to_string(page_id);

            if (idx == access_index - 1) {
                // 当前访问的页面
                if (memory_set.find(page_id) != memory_set.end()) {
                    // 命中
                    ImGui::PushStyleColor(ImGuiCol_Button, hit_color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
                }
                else {
                    // 缺页
                    ImGui::PushStyleColor(ImGuiCol_Button, fault_color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
                }

                ImGui::Button(label.c_str(), { 35, 35 });
                ImGui::PopStyleColor(3);
            }
            else if (idx < access_index) {
                // 已访问过的页面
                if (memory_set.find(page_id) != memory_set.end()) {
                    // 当时在内存中
                    ImGui::PushStyleColor(ImGuiCol_Button, memory_color);
                }
                else {
                    // 当时不在内存中
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                }

                ImGui::Button(label.c_str(), { 35, 35 });
                ImGui::PopStyleColor(1);
            }
            else {
                // 未访问的页面
                ImGui::Button(label.c_str(), { 35, 35 });
            }

            ImGui::PopItemWidth();
        }
    }

    ImGui::EndChild();

    // 图例
    ImGui::Spacing();
    ImGui::Text(u8"图例:");
    ImGui::SameLine();
    ImGui::TextColored(hit_color, u8"■ 命中");
    ImGui::SameLine();
    ImGui::TextColored(fault_color, u8"■ 缺页");
    ImGui::SameLine();
    ImGui::TextColored(memory_color, u8"■ 在内存中");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), u8"■ 不在内存中");
}

// 绘制统计信息
void InAndOut::draw_statistics()
{
    ImGui::Columns(2, u8"统计信息列", false);

    // 命中信息
    ImGui::Text(u8"命中次数:");
    ImGui::SameLine();
    ImGui::TextColored(hit_color, "%d", hit_count);

    ImGui::Text(u8"命中率:");
    ImGui::SameLine();
    ImGui::TextColored(hit_color, "%.2f%%", hit_rate);

    // 进度条显示命中率
    ImGui::ProgressBar(hit_rate / 100.0f, { 150, 20 }, "");

    ImGui::NextColumn();

    // 缺页信息
    ImGui::Text(u8"缺页次数:");
    ImGui::SameLine();
    ImGui::TextColored(fault_color, "%d", fault_count);

    ImGui::Text(u8"缺页率:");
    ImGui::SameLine();
    ImGui::TextColored(fault_color, "%.2f%%", fault_rate);

    // 进度条显示缺页率
    ImGui::ProgressBar(fault_rate / 100.0f, { 150, 20 }, "");

    ImGui::Columns(1);

    ImGui::Spacing();

    // 总访问次数
    ImGui::Text(u8"总访问次数: %d", total_access);

    // 算法效率评估
    ImGui::Spacing();
    if (total_access > 0) {
        std::string efficiency;
        if (hit_rate > 70.0f) {
            efficiency = u8"优秀";
        }
        else if (hit_rate > 50.0f) {
            efficiency = u8"良好";
        }
        else if (hit_rate > 30.0f) {
            efficiency = u8"一般";
        }
        else {
            efficiency = u8"较差";
        }

        ImGui::Text(u8"算法效率: %s", efficiency.c_str());
    }
}

//总访问次数 序列

//物理内存状态