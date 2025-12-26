#include "11.thread_mechanism.h"

void ThreadMechanism::on_enter()
{
    str_tip_list.clear();
    process_idx = 1;
    sum_time = 0;
    current_pcb = nullptr;
    can_execute = false;
    is_executing = false;

    pcb_queue_normal.clear();
    pcb_queue_can_execute.clear();

    // 线程相关初始化
    thread_queue.clear();
    ready_queue.clear();
    blocked_queue.clear();
    terminated_queue.clear();
    current_thread = nullptr;

    current_algorithm = 0;
    time_quantum = 0.5f;
    thread_id_counter = 1;
    rr_timer = 0.0f;

    total_threads_created = 0;
    total_threads_completed = 0;
    total_wait_time = 0.0f;
    total_turnaround_time = 0.0f;

    // 初始化随机种子
    srand(static_cast<unsigned>(time(nullptr)));
}

void ThreadMechanism::on_exit()
{
    ConfigManager::instance()->clear_resource();
    pcb_queue_normal.clear();
    pcb_queue_can_execute.clear();
    str_tip_list.clear();

    thread_queue.clear();
    ready_queue.clear();
    blocked_queue.clear();
    terminated_queue.clear();
    current_thread = nullptr;
}

void ThreadMechanism::on_input(const SDL_Event* event)
{
    if (ImGui::GetIO().WantCaptureKeyboard) return;
    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_e)
    {
        // 快捷键 E: 开始/暂停执行
        is_executing = !is_executing;
        can_execute = is_executing;
        std::string tip = std::to_string(sum_time) + "---" +
            (is_executing ? "开始线程调度" : "暂停线程调度");
        str_tip_list.push_back(TextString(tip, TextType::Info));
    }
    else if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_r)
    {
        // 快捷键 R: 重置模拟
        reset_simulation();
    }
}

void ThreadMechanism::on_update(float delta)
{
    on_update_imgui_region1(delta);
    on_update_imgui_region2(delta);
    on_update_imgui_region3(delta);
    on_update_imgui_region4(delta);

    if (!can_execute) return;
    sum_time += delta;
    on_update_process(delta);
}

void ThreadMechanism::on_update_imgui_region1(float delta)
{
    // 调度算法选择区域
    ImGui::BeginChild("thread_region1", { ImGui::GetContentRegionAvail().x, 100 });

    ImGui::Text(u8"调度算法选择");
    ImGui::Separator();

    // 显示当前算法
    ImGui::Text(u8"当前算法: %s", algorithm_names[current_algorithm].c_str());
    ImGui::SameLine();
    if (current_algorithm == 3) { // RR算法
        ImGui::Text(u8"(时间片: %.1f秒)", time_quantum);
    }

    // 算法选择按钮
    float button_width = (ImGui::GetContentRegionAvail().x - 30) / 4.0f;

    for (int i = 0; i < 4; i++) {
        if (i > 0) ImGui::SameLine();
        bool is_selected = (current_algorithm == i);
        if (is_selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
        }

        if (ImGui::Button(algorithm_names[i].c_str(), ImVec2(button_width, 40))) {
            switch_scheduling_algorithm(i);
        }

        if (is_selected) {
            ImGui::PopStyleColor(2);
        }
    }

    // 时间片设置（仅RR算法时显示）
    if (current_algorithm == 3) {
        ImGui::Spacing();
        ImGui::Text(u8"时间片大小:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        if (ImGui::SliderFloat("##time_quantum", &time_quantum, 0.1f, 2.0f, u8"%.1f秒")) {
            time_quantum = std::max(0.1f, time_quantum);
            rr_timer = 0.0f; // 重置计时器
        }
        ImGui::SameLine();
        ImGui::Text(u8"(小时间片响应快，大时间片吞吐量高)");
    }

    ImGui::EndChild();
    ImGui::Separator();
}

void ThreadMechanism::on_update_imgui_region2(float delta)
{
    // 进程与线程管理区域
    ImGui::BeginChild("thread_region2", { ImGui::GetContentRegionAvail().x * 0.6f, 350 });

    ImGui::Text(u8"进程与线程管理");
    ImGui::Separator();

    // 创建进程按钮
    ImGui::BeginGroup();
    if (ImGui::Button(u8"创建新进程", { ImGui::GetContentRegionAvail().x * 0.49f, 40 }))
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
            pcb->pname = u8"进程" + std::to_string(process_idx);
            pcb->update_state(ProcessState::ActiveBlock);

            // 为进程创建线程
            int thread_count = 2 + (rand() % 3); // 2-4个线程
            create_thread_for_process(pcb, thread_count);

            pcb->timer_execute.set_wait_time(2.0);
            pcb->timer_execute.set_one_shot(true);
            pcb->timer_execute.set_on_timeout([&, pcb]() {
                pcb->update_state(ProcessState::Finish);
                std::string tip = std::to_string(sum_time) + u8"---进程结束: " + pcb->pname;
                str_tip_list.push_back(TextString(tip, TextType::Info));
                });

            std::string tip = std::to_string(sum_time) + u8"---进程创建: " + pcb->pname +
                u8" (包含" + std::to_string(thread_count) + u8"个线程)";
            str_tip_list.push_back(TextString(tip, TextType::Info));
            pcb_queue_normal.push_back(pcb);
            process_idx++;
        }
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"创建独立线程", { ImGui::GetContentRegionAvail().x, 40 })) {
        // 创建不属于任何进程的独立线程
        int tid = thread_id_counter++;
        std::string tname = u8"独立线程" + std::to_string(tid);

        auto thread = std::make_shared<TCB>(tid, tname, nullptr);
        thread_queue.push_back(thread);
        ready_queue.push_back(thread);
        total_threads_created++;

        std::string tip = std::to_string(sum_time) + u8"---创建独立线程: " + tname;
        str_tip_list.push_back(TextString(tip, TextType::Info));
    }
    ImGui::EndGroup();

    // 进程列表显示
    ImGui::Spacing();
    ImGui::Text(u8"进程列表:");
    ImGui::BeginChild("process_list", { 0, 180 }, true);

    if (pcb_queue_normal.empty()) {
        ImGui::TextDisabled(u8"暂无进程，请先创建进程");
    }
    else {
        for (int i = 0; i < pcb_queue_normal.size(); i++) {
            auto& pcb = pcb_queue_normal[i];

            // 计算进程中的线程数
            int thread_count = 0;
            int running_threads = 0;
            for (auto& thread : thread_queue) {
                if (thread->parent_process == pcb) {
                    thread_count++;
                    if (thread->state == ThreadState::Running) {
                        running_threads++;
                    }
                }
            }

            std::string label = pcb->pname;
            if (thread_count > 0) {
                label += u8" (线程: " + std::to_string(running_threads) +
                    u8"/" + std::to_string(thread_count) + u8")";
            }

            bool is_selected = (current_pcb == pcb.get());
            if (is_selected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 0.7f));
            }

            if (ImGui::Button(label.c_str(), { ImGui::GetContentRegionAvail().x * 0.7f, 30 })) {
                current_pcb = pcb.get();
                selected_process = i;
            }

            if (is_selected) {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();

            std::string btn_id = u8"添加##" + std::to_string(i);
            if (ImGui::Button(btn_id.c_str(), { ImGui::GetContentRegionAvail().x, 30 })) {
                selected_process = i;
                current_pcb = pcb.get();
                create_thread_for_process(pcb, 1);
            }

            // 显示进程详细信息（悬停提示）
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text(u8"进程详细信息:");
                ImGui::Separator();
                ImGui::Text(u8"名称: %s", pcb->pname.c_str());
                ImGui::Text(u8"状态: %s", pcb->state_name.c_str());
                ImGui::Text(u8"线程数: %d", thread_count);

                if (thread_count > 0) {
                    ImGui::Separator();
                    ImGui::Text(u8"线程状态:");
                    for (auto& thread : thread_queue) {
                        if (thread->parent_process == pcb) {
                            std::string state_str;
                            switch (thread->state) {
                            case ThreadState::Running: state_str = u8"运行"; break;
                            case ThreadState::Ready: state_str = u8"就绪"; break;
                            case ThreadState::Blocked: state_str = u8"阻塞"; break;
                            case ThreadState::Terminated: state_str = u8"终止"; break;
                            }
                            ImGui::BulletText(u8"%s: %s (%.1f/%.1f秒)",
                                thread->tname.c_str(), state_str.c_str(),
                                thread->time_executed, thread->time_needed);
                        }
                    }
                }
                ImGui::EndTooltip();
            }
        }
    }

    ImGui::EndChild();

    // 线程操作按钮
    ImGui::Spacing();
    ImGui::Text(u8"线程操作:");
    ImGui::BeginGroup();

    if (ImGui::Button(u8"阻塞当前线程", { ImGui::GetContentRegionAvail().x * 0.49f, 30 })) {
        if (current_thread) {
            current_thread->state = ThreadState::Blocked;
            blocked_queue.push_back(current_thread);
            std::string tip = std::to_string(sum_time) + u8"---线程阻塞: " + current_thread->tname;
            str_tip_list.push_back(TextString(tip, TextType::Warning));
            current_thread = nullptr;
        }
        else {
            str_tip_list.push_back(TextString(u8"没有正在运行的线程", TextType::Error));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"结束当前线程", { ImGui::GetContentRegionAvail().x, 30 })) {
        if (current_thread) {
            current_thread->state = ThreadState::Terminated;
            current_thread->turnaround_time = sum_time;
            total_turnaround_time += current_thread->turnaround_time;
            total_threads_completed++;
            terminated_queue.push_back(current_thread);

            std::string tip = std::to_string(sum_time) + u8"---线程终止: " + current_thread->tname;
            str_tip_list.push_back(TextString(tip, TextType::Info));
            current_thread = nullptr;
        }
        else {
            str_tip_list.push_back(TextString(u8"没有正在运行的线程", TextType::Error));
        }
    }
    ImGui::EndGroup();

    ImGui::EndChild();
    ImGui::SameLine();

    // 线程状态区域
    ImGui::BeginChild("thread_status", { 0, 350 }, true);
    draw_thread_info();
    ImGui::EndChild();
    ImGui::Separator();
}

void ThreadMechanism::on_update_imgui_region3(float delta)
{
    // 线程调度控制区域
    ImGui::BeginChild("thread_region3", { ImGui::GetContentRegionAvail().x, 100 });

    ImGui::Text(u8"线程调度控制");
    ImGui::Separator();

    // 控制按钮
    ImGui::BeginGroup();

    if (ImGui::Button(is_executing ? u8"暂停调度" : u8"开始调度", { 120, 40 })) {
        is_executing = !is_executing;
        can_execute = is_executing;
        std::string tip = std::to_string(sum_time) + u8"---" +
            (is_executing ? "开始线程调度" : "暂停线程调度");
        str_tip_list.push_back(TextString(tip, TextType::Info));
    }

    ImGui::SameLine();

    //if (ImGui::Button(u8"单步执行", { 120, 40 })) {
    //    if (!is_executing) {
    //        can_execute = true;
    //        on_update_process(0.1f); // 执行一小步
    //        can_execute = false;
    //        std::string tip = std::to_string(sum_time) + u8"---单步执行完成";
    //        str_tip_list.push_back(TextString(tip, TextType::Info));
    //    }
    //    else {
    //        str_tip_list.push_back(TextString(u8"请先暂停调度再进行单步执行", TextType::Error));
    //    }
    //}

    //ImGui::SameLine();

    if (ImGui::Button(u8"重置模拟", { 120, 40 })) {
        reset_simulation();
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"清空日志", { 120, 40 })) {
        str_tip_list.clear();
    }

    ImGui::EndGroup();

    // 显示当前执行线程和统计信息
    ImGui::Spacing();
    if (current_thread) {
        float progress = current_thread->time_executed / current_thread->time_needed;
        std::string progress_text = u8"当前执行: " + current_thread->tname +
            u8" (" + std::to_string(static_cast<int>(progress * 100)) + u8"%)";
        ImGui::Text("%s", progress_text.c_str());
        ImGui::SameLine();
        ImGui::ProgressBar(progress, ImVec2(200, 20));

        ImGui::Text(u8"已执行: %.1f秒 / 总需: %.1f秒",
            current_thread->time_executed, current_thread->time_needed);
    }
    else {
        ImGui::TextDisabled(u8"当前没有正在执行的线程");
    }

    ImGui::EndChild();
    ImGui::Separator();
}

void ThreadMechanism::on_update_imgui_region4(float delta)
{
    // 信息显示区域
    ImGui::BeginChild("thread_region4", { ImGui::GetContentRegionAvail().x, 0 }, true);

    ImGui::Text(u8"运行信息");
    ImGui::Separator();

    // 显示统计信息
    ImGui::Text(u8"统计信息:");
    ImGui::Text(u8"  总线程数: %d", total_threads_created);
    ImGui::Text(u8"  已完成: %d", total_threads_completed);
    if (total_threads_completed > 0) {
        ImGui::Text(u8"  平均等待时间: %.2f秒", total_wait_time / total_threads_completed);
        ImGui::Text(u8"  平均周转时间: %.2f秒", total_turnaround_time / total_threads_completed);
    }
    ImGui::Text(u8"  当前就绪队列: %d", ready_queue.size());
    ImGui::Text(u8"  当前阻塞队列: %d", blocked_queue.size());

    ImGui::Separator();
    ImGui::Text(u8"运行日志:");

    // 显示str_tip_list中的所有提示
    ImGui::BeginChild("log_list", { 0, 80 }, true);
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

    ImGui::EndChild();
}

void ThreadMechanism::on_update_process(float delta)
{
    if (!is_executing) return;

    // 更新所有进程的定时器
    for (auto& pcb : pcb_queue_normal) {
        pcb->timer_execute.on_update(delta);
    }

    // 更新所有就绪线程的等待时间
    for (auto& thread : ready_queue) {
        if (thread->state == ThreadState::Ready) {
            thread->wait_time += delta;
        }
    }

    // 执行当前线程
    if (current_thread && current_thread->state == ThreadState::Running) {
        execute_thread(delta);
    }
    else {
        // 如果没有当前线程或当前线程已结束，立即调度
        schedule_threads();
    }

    // 更新线程状态（处理阻塞恢复等）
    update_thread_states();

    // 对于RR算法，检查时间片是否用完
    if (current_algorithm == 3 && current_thread) { // RR算法
        rr_timer += delta;
        if (rr_timer >= time_quantum) {
            // 时间片用完，强制切换
            if (current_thread->state == ThreadState::Running) {
                current_thread->state = ThreadState::Ready;
                ready_queue.push_back(current_thread);
                std::string tip = std::to_string(sum_time) + u8"---时间片用完: " +
                    current_thread->tname + u8" 切换到就绪队列";
                str_tip_list.push_back(TextString(tip, TextType::Warning));
            }
            current_thread = nullptr;
            rr_timer = 0.0f;
            schedule_threads();
        }
    }
}

void ThreadMechanism::create_thread_for_process(std::shared_ptr<PCB> process, int thread_count)
{
    for (int i = 0; i < thread_count; i++) {
        int tid = thread_id_counter++;
        std::string tname = u8"线程" + std::to_string(tid) + u8"@" + process->pname;

        auto thread = std::make_shared<TCB>(tid, tname, process);

        // 设置随机参数
        thread->time_needed = 1.0f + static_cast<float>(rand() % 150) / 100.0f; // 1.0-2.5秒
        thread->priority = 1 + rand() % 3; // 优先级1-3
        thread->time_remaining = thread->time_needed;

        thread_queue.push_back(thread);
        ready_queue.push_back(thread);
        total_threads_created++;

        std::string tip = std::to_string(sum_time) + u8"---创建线程: " + tname +
            u8" (优先级:" + std::to_string(thread->priority) +
            u8", 需时:" + std::to_string(thread->time_needed).substr(0, 3) + u8"秒)";
        str_tip_list.push_back(TextString(tip, TextType::Info));
    }
}

void ThreadMechanism::schedule_threads()
{
    // 如果没有当前线程或当前线程已结束/阻塞，则进行调度
    if (!current_thread ||
        current_thread->state != ThreadState::Running) {

        // 如果就绪队列为空，尝试从阻塞队列恢复一些线程
        if (ready_queue.empty() && !blocked_queue.empty()) {
            // 随机恢复一些阻塞的线程
            for (auto it = blocked_queue.begin(); it != blocked_queue.end();) {
                if (rand() % 5 == 0) { // 20%概率恢复
                    (*it)->state = ThreadState::Ready;
                    ready_queue.push_back(*it);
                    std::string tip = std::to_string(sum_time) + u8"---线程恢复: " + (*it)->tname;
                    str_tip_list.push_back(TextString(tip, TextType::Info));
                    it = blocked_queue.erase(it);
                }
                else {
                    ++it;
                }
            }
        }

        // 如果就绪队列仍然为空，返回
        if (ready_queue.empty()) {
            return;
        }

        std::shared_ptr<TCB> next_thread = nullptr;
        int selected_index = 0;

        // 根据调度算法选择下一个线程
        switch (current_algorithm) {
        case 0: // FCFS - 先来先服务
            next_thread = ready_queue[0];
            break;

        case 1: // SJF - 最短作业优先
            for (size_t i = 0; i < ready_queue.size(); i++) {
                auto& thread = ready_queue[i];
                if (!next_thread || thread->time_remaining < next_thread->time_remaining) {
                    next_thread = thread;
                    selected_index = i;
                }
            }
            break;

        case 2: // Priority - 优先级调度（数字越小优先级越高）
            for (size_t i = 0; i < ready_queue.size(); i++) {
                auto& thread = ready_queue[i];
                if (!next_thread || thread->priority < next_thread->priority) {
                    next_thread = thread;
                    selected_index = i;
                }
            }
            break;

        case 3: // RR - 时间片轮转
            next_thread = ready_queue[0];
            selected_index = 0;
            break;
        }

        // 切换到新线程
        if (next_thread && next_thread->state == ThreadState::Ready) {
            // 从就绪队列中移除
            ready_queue.erase(ready_queue.begin() + selected_index);

            // 更新旧线程状态
            if (current_thread && current_thread->state == ThreadState::Running) {
                current_thread->state = ThreadState::Ready;
                ready_queue.push_back(current_thread);
            }

            // 设置新线程
            current_thread = next_thread;
            current_thread->state = ThreadState::Running;

            // 重置RR计时器
            rr_timer = 0.0f;

            std::string tip = std::to_string(sum_time) + u8"---调度线程: " + current_thread->tname;
            str_tip_list.push_back(TextString(tip, TextType::Info));
        }
    }
}

void ThreadMechanism::execute_thread(float delta)
{
    if (!current_thread) return;

    // 执行线程
    current_thread->time_executed += delta;
    current_thread->time_remaining -= delta;

    // 检查线程是否完成
    if (current_thread->time_remaining <= 0.0f) {
        current_thread->state = ThreadState::Terminated;
        current_thread->turnaround_time = sum_time;

        // 更新统计信息
        total_wait_time += current_thread->wait_time;
        total_turnaround_time += current_thread->turnaround_time;
        total_threads_completed++;

        terminated_queue.push_back(current_thread);

        std::string tip = std::to_string(sum_time) + u8"---线程完成: " + current_thread->tname;
        str_tip_list.push_back(TextString(tip, TextType::KeyInfo));

        // 随机模拟线程阻塞（15%概率）
        if (rand() % 100 < 15 && current_thread->state != ThreadState::Terminated) {
            current_thread->state = ThreadState::Blocked;
            blocked_queue.push_back(current_thread);
            terminated_queue.pop_back(); // 从终止队列移除

            std::string tip = std::to_string(sum_time) + u8"---线程阻塞: " + current_thread->tname;
            str_tip_list.push_back(TextString(tip, TextType::Warning));
        }

        current_thread = nullptr;
    }
}

void ThreadMechanism::update_thread_states()
{
    // 随机检查阻塞队列中的线程是否恢复
    for (auto it = blocked_queue.begin(); it != blocked_queue.end();) {
        // 随机恢复概率（每帧5%）
        if (rand() % 20 == 0) {
            (*it)->state = ThreadState::Ready;
            ready_queue.push_back(*it);

            std::string tip = std::to_string(sum_time) + u8"---线程恢复: " + (*it)->tname;
            str_tip_list.push_back(TextString(tip, TextType::Info));

            it = blocked_queue.erase(it);
        }
        else {
            ++it;
        }
    }

    // 清理已完成的线程
    for (auto it = thread_queue.begin(); it != thread_queue.end();) {
        if ((*it)->state == ThreadState::Terminated) {
            // 计算线程生命周期
            float lifetime = (*it)->turnaround_time;
            (*it)->turnaround_time = lifetime;

            it = thread_queue.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ThreadMechanism::draw_thread_info()
{
    ImGui::Text(u8"线程状态统计");
    ImGui::Separator();

    // 统计信息
    int total = thread_queue.size();
    int ready = 0;
    int running = 0;
    int blocked = 0;
    int terminated = terminated_queue.size();

    for (auto& thread : thread_queue) {
        switch (thread->state) {
        case ThreadState::Running: running++; break;
        case ThreadState::Ready: ready++; break;
        case ThreadState::Blocked: blocked++; break;
        default: break;
        }
    }

    // 显示统计图表
    ImGui::Text(u8"总计: %d  就绪: %d  运行: %d  阻塞: %d  完成: %d",
        total, ready, running, blocked, terminated);

    // 进度条显示状态分布
    if (total > 0) {
        float ready_ratio = static_cast<float>(ready) / total;
        float running_ratio = static_cast<float>(running) / total;
        float blocked_ratio = static_cast<float>(blocked) / total;
        float terminated_ratio = static_cast<float>(terminated) / (total + terminated);

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // 运行-绿色
        ImGui::Text(u8"运行: ");
        ImGui::SameLine();
        ImGui::ProgressBar(running_ratio, ImVec2(100, 15));

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // 就绪-黄色
        ImGui::Text(u8"就绪: ");
        ImGui::SameLine();
        ImGui::ProgressBar(ready_ratio, ImVec2(100, 15));

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // 阻塞-红色
        ImGui::Text(u8"阻塞: ");
        ImGui::SameLine();
        ImGui::ProgressBar(blocked_ratio, ImVec2(100, 15));

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // 完成-灰色
        ImGui::Text(u8"完成: ");
        ImGui::SameLine();
        ImGui::ProgressBar(terminated_ratio, ImVec2(100, 15));

        ImGui::PopStyleColor(4);
    }

    ImGui::Separator();
    ImGui::Text(u8"线程详情:");

    // 显示所有线程
    ImGui::BeginChild("thread_detail_list", { 0, 0 }, true);
    for (auto& thread : thread_queue) {
        std::string state_str;
        ImVec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

        switch (thread->state) {
        case ThreadState::Running:
            state_str = u8"运行";
            color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
            break;
        case ThreadState::Ready:
            state_str = u8"就绪";
            color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
            break;
        case ThreadState::Blocked:
            state_str = u8"阻塞";
            color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            break;
        default:
            state_str = u8"未知";
        }

        // 显示线程信息
        std::string thread_info = thread->tname + u8" [" + state_str + u8"]";
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << thread->time_executed << "/" << thread->time_needed;
        std::string detail_info = u8"P:"+std::to_string(thread->priority) + u8" 进度:" + oss.str();

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::Text("%s", thread_info.c_str());
        ImGui::PopStyleColor();
        //@
        ImGui::SameLine(135);
        ImGui::TextDisabled("%s", detail_info.c_str());

        // 进度条
        float progress = thread->time_executed / thread->time_needed;
        ImGui::ProgressBar(progress, ImVec2(-1, 4));
    }
    ImGui::EndChild();
}

void ThreadMechanism::switch_scheduling_algorithm(int algorithm)
{
    if (algorithm >= 0 && algorithm < 4) {
        current_algorithm = algorithm;

        // 重置RR计时器
        rr_timer = 0.0f;

        // 如果当前有线程在运行，根据新算法可能需要重新调度
        if (current_thread && current_thread->state == ThreadState::Running) {
            current_thread->state = ThreadState::Ready;
            ready_queue.insert(ready_queue.begin(), current_thread);
            current_thread = nullptr;
        }

        std::string tip = std::to_string(sum_time) + u8"---切换调度算法: " + algorithm_names[algorithm];
        str_tip_list.push_back(TextString(tip, TextType::KeyInfo));
    }
}

void ThreadMechanism::reset_simulation()
{
    str_tip_list.clear();
    process_idx = 1;
    sum_time = 0;
    current_pcb = nullptr;
    can_execute = false;
    is_executing = false;

    pcb_queue_normal.clear();
    pcb_queue_can_execute.clear();

    thread_queue.clear();
    ready_queue.clear();
    blocked_queue.clear();
    terminated_queue.clear();
    current_thread = nullptr;

    thread_id_counter = 1;
    rr_timer = 0.0f;

    total_threads_created = 0;
    total_threads_completed = 0;
    total_wait_time = 0.0f;
    total_turnaround_time = 0.0f;

    str_tip_list.push_back(TextString(u8"模拟已重置", TextType::Info));
}

//先来先服务//终止
//随机
//日志
//优先级
//单步执行
//时间片
//state_name  150
//结束并统计