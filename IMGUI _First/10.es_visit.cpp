// 10.es_visit.cpp (完整实现)
#include "10.es_visit.h"

void EsVisit::on_enter()
{
    str_tip_list.clear();
    sum_time = 0;
    algo_choose_id = 0;
    can_execute = false;

    // 生成初始访问序列
    generate_track_sequence();
}

void EsVisit::on_exit()
{
    ConfigManager::instance()->clear_resource();
}

void EsVisit::on_update(float delta)
{
    on_update_imgui_region1(delta);
    on_update_imgui_region2(delta);
    on_update_imgui_region3(delta);
    on_update_imgui_region4(delta);
    if (!can_execute) return;
    sum_time += delta;
    on_update_process(delta);
}

void EsVisit::on_update_imgui_region1(float delta)
{
    ImGui::BeginChild("process_communication_region1", { ImGui::GetContentRegionAvail().x, 140 });

    ImGui::Text(u8"算法选择"); ImGui::Separator();
    ImGui::RadioButton(u8"FCFS（先来先服务）", &algo_choose_id, 0); ImGui::SameLine();
    ImGui::RadioButton(u8"SSTF（最短寻道时间优先）", &algo_choose_id, 0);
    ImGui::Separator();

    ImGui::Text(u8"磁道信息配置");
    ImGui::SliderInt(u8"起始位置", &start_pos, 0, max_track);
    ImGui::SliderInt(u8"最大磁道", &max_track, 100, 500);
    ImGui::SliderInt(u8"访问序列大小", &sequence_size, 5, 20);

    ImGui::Separator();

    // 控制按钮
    if (ImGui::Button(u8"生成访问序列", { ImGui::GetContentRegionAvail().x / 3, 30 })) {
        generate_track_sequence();
    }
    ImGui::SameLine();

    if (ImGui::Button(u8"执行算法", { ImGui::GetContentRegionAvail().x / 2, 30 })) {
        can_execute = true;

        // 执行两个算法
        execute_fcfs();
        execute_sstf();

        str_tip_list.push_back(TextString(u8"磁盘访问执行结束...", TextType::KeyInfo));
    }
    ImGui::SameLine();

    if (ImGui::Button(u8"重置", { ImGui::GetContentRegionAvail().x, 30 })) {
        reset_simulation();
        can_execute = false;
    }

    ImGui::Separator();

    // 显示当前访问序列
    ImGui::Text(u8"当前访问序列:");
    std::string track_str = "[";
    for (size_t i = 0; i < track_list.size(); i++) {
        track_str += std::to_string(track_list[i]);
        if (i < track_list.size() - 1) track_str += ", ";
    }
    track_str += "]";
    ImGui::Text("%s", track_str.c_str());

    ImGui::EndChild();
    ImGui::Separator();
}

void EsVisit::on_update_imgui_region2(float delta)
{
    ImGui::BeginChild("process_communication_region2", { ImGui::GetContentRegionAvail().x / 2, 360 }, 1);

    ImGui::Text(u8"FCFS算法结果");
    ImGui::Separator();

    // 显示FCFS结果表格
    if (ImGui::BeginTable("FCFS_Table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn(u8"序号");
        ImGui::TableSetupColumn(u8"访问磁道");
        ImGui::TableSetupColumn(u8"移动距离");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < fcfs_result.sequence.size(); i++) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", i + 1);

            ImGui::TableNextColumn();
            ImGui::Text("%d", fcfs_result.sequence[i]);

            ImGui::TableNextColumn();
            ImGui::Text("%d", fcfs_result.distances[i]);
        }

        ImGui::EndTable();
    }

    // 显示平均寻道长度
    ImGui::Separator();
    ImGui::Text(u8"平均寻道长度: %.2f", fcfs_result.avg_seek_length);

    ImGui::EndChild();
    ImGui::SameLine();
}

void EsVisit::on_update_imgui_region3(float delta)
{
    ImGui::BeginChild("process_communication_region3", { ImGui::GetContentRegionAvail().x, 360 }, 1);

    ImGui::Text(u8"SSTF算法结果");
    ImGui::Separator();

    // 显示SSTF结果表格
    if (ImGui::BeginTable("SSTF_Table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn(u8"序号");
        ImGui::TableSetupColumn(u8"访问磁道");
        ImGui::TableSetupColumn(u8"移动距离");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < sstf_result.sequence.size(); i++) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", i + 1);

            ImGui::TableNextColumn();
            ImGui::Text("%d", sstf_result.sequence[i]);

            ImGui::TableNextColumn();
            ImGui::Text("%d", sstf_result.distances[i]);
        }

        ImGui::EndTable();
    }

    // 显示平均寻道长度
    ImGui::Separator();
    ImGui::Text(u8"平均寻道长度: %.2f", sstf_result.avg_seek_length);

    ImGui::EndChild();
    ImGui::Separator();
}

void EsVisit::on_update_imgui_region4(float delta)
{
    ImGui::BeginChild("process_communication_region4", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y}, true);

    if (ImGui::Button(u8"清空日志", { ImGui::GetContentRegionAvail().x, 30 })) {
        str_tip_list.clear();
    }
    ImGui::Separator();

    // 显示str_tip_list中的所有提示
    for (const auto& textString : str_tip_list) {
        ImGui::PushStyleColor(ImGuiCol_Text, static_cast<ImVec4>(ConfigManager::instance()->get_text_color(textString.type)));
        ImGui::TextUnformatted(textString.text_info.c_str());
        ImGui::PopStyleColor();
    }

    static size_t last_tip_count = 0;
    if (last_tip_count != str_tip_list.size()) {
        ImGui::SetScrollY(ImGui::GetScrollMaxY() + 2000);
        last_tip_count = str_tip_list.size();
    }

    ImGui::EndChild();
}

void EsVisit::on_update_process(float delta)
{
    // 这里可以添加逐步模拟的逻辑
    // 目前我们是一次性计算并显示结果
    // 如果需要逐步动画，可以在这里添加状态机逻辑

}


// 生成随机访问序列
void EsVisit::generate_track_sequence()
{
    track_list.clear();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, max_track);

    for (int i = 0; i < sequence_size; i++) {
        track_list.push_back(dis(gen));
    }

    std::stringstream ss;
    ss << u8"已生成访问序列：[";
    for (size_t i = 0; i < track_list.size(); i++) {
        ss << track_list[i];
        if (i < track_list.size() - 1) ss << ", ";
    }
    ss << "]";

    str_tip_list.push_back(TextString(ss.str(), TextType::Info));
}

// 执行FCFS算法
void EsVisit::execute_fcfs()
{
    TextString log_entry(u8"FCFS算法执行中...", TextType::KeyInfo);
    str_tip_list.push_back(log_entry);
    fcfs_result.sequence.clear();
    fcfs_result.distances.clear();

    int current = start_pos;
    int total_distance = 0;

    for (int track : track_list) {
        int distance = abs(track - current);
        fcfs_result.sequence.push_back(track);
        fcfs_result.distances.push_back(distance);
        total_distance += distance;
        current = track;
        TextString log_entry(
            u8"访问磁道: " + std::to_string(track) +
            u8", 移动距离: " + std::to_string(distance),
            TextType::Info);
        str_tip_list.push_back(log_entry);
    }

    fcfs_result.avg_seek_length = track_list.empty() ? 0 :
        static_cast<float>(total_distance) / track_list.size();

    // 记录日志
    std::stringstream ss;
    ss << u8"FCFS算法执行完成，平均寻道长度: "
        << std::fixed << std::setprecision(2) << fcfs_result.avg_seek_length;
    str_tip_list.push_back(TextString(ss.str(), TextType::Finish));
}

// 执行SSTF算法
void EsVisit::execute_sstf()
{
    TextString log_entry(u8"SSTF算法执行中...", TextType::KeyInfo);
    str_tip_list.push_back(log_entry);

    sstf_result.sequence.clear();
    sstf_result.distances.clear();

    int current = start_pos;
    std::vector<int> remaining_tracks = track_list;
    int total_distance = 0;

    while (!remaining_tracks.empty()) {
        // 找到距离最近的磁道
        int nearest_idx = 0;
        int min_distance = abs(remaining_tracks[0] - current);

        for (size_t i = 1; i < remaining_tracks.size(); i++) {
            int distance = abs(remaining_tracks[i] - current);
            if (distance < min_distance) {
                min_distance = distance;
                nearest_idx = i;
            }
        }

        // 访问该磁道
        int next_track = remaining_tracks[nearest_idx];
        sstf_result.sequence.push_back(next_track);  // 修正：改为 sstf_result
        sstf_result.distances.push_back(min_distance);  // 修正：改为 sstf_result
        total_distance += min_distance;

        // 更新当前位置并移除已访问的磁道
        current = next_track;
        remaining_tracks.erase(remaining_tracks.begin() + nearest_idx);
        TextString log_entry(
            u8"访问磁道: " + std::to_string(next_track) +
            u8", 移动距离: " + std::to_string(min_distance),
            TextType::Info);
        str_tip_list.push_back(log_entry);
    }

    sstf_result.avg_seek_length = track_list.empty() ? 0 :
        static_cast<float>(total_distance) / track_list.size();

    // 记录日志
    std::stringstream ss;
    ss << u8"SSTF算法执行完成，平均寻道长度: "
        << std::fixed << std::setprecision(2) << sstf_result.avg_seek_length;
    str_tip_list.push_back(TextString(ss.str(), TextType::Finish));
}

// 重置模拟
void EsVisit::reset_simulation()
{
    fcfs_result = AlgorithmResult();
    sstf_result = AlgorithmResult();
    track_list.clear();
    str_tip_list.push_back(TextString(u8"模拟已重置", TextType::Info));
}