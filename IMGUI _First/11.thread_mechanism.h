#pragma once

#include"almighty_head.h"

class ThreadMechanism : public Example
{
public:
    void on_enter() override;
    void on_update(float delta) override;
    void on_input(const SDL_Event* event) override;
    void on_exit() override;

private:
    void on_update_imgui_region1(float delta); // 调度算法选择
    void on_update_imgui_region2(float delta); // 进程与线程管理
    void on_update_imgui_region3(float delta); // 线程调度控制
    void on_update_imgui_region4(float delta); // 信息显示
    void on_update_process(float delta);       // 进程和线程更新

    // 线程相关方法
    void create_thread_for_process(std::shared_ptr<PCB> process, int thread_count = 1);
    void schedule_threads();
    void execute_thread(float delta);
    void update_thread_states();
    void draw_thread_info();
    void switch_scheduling_algorithm(int algorithm);
    void reset_simulation();

private:
    float sum_time = 0;
    int process_idx = 1;
    bool can_execute = false;
    bool is_executing = false;

    PCB* current_pcb = nullptr;
    std::vector<std::shared_ptr<PCB>> pcb_queue_normal;
    std::vector<std::shared_ptr<PCB>> pcb_queue_can_execute;

    std::string str_tip = "";
    std::vector<TextString> str_tip_list;

    int selected_process = 0;

    // 线程相关成员
    std::vector<std::shared_ptr<TCB>> thread_queue;      // 所有线程队列
    std::vector<std::shared_ptr<TCB>> ready_queue;       // 就绪队列
    std::vector<std::shared_ptr<TCB>> blocked_queue;     // 阻塞队列
    std::vector<std::shared_ptr<TCB>> terminated_queue;  // 终止队列
    std::shared_ptr<TCB> current_thread;                 // 当前执行线程

    int current_algorithm = 0;    // 当前调度算法: 0-FCFS, 1-SJF, 2-Priority, 3-RR
    float time_quantum = 0.5f;    // 时间片大小（用于RR算法）
    int thread_id_counter = 1;    // 线程ID计数器
    float rr_timer = 0.0f;        // RR算法计时器

    // 统计信息
    int total_threads_created = 0;
    int total_threads_completed = 0;
    float total_wait_time = 0.0f;
    float total_turnaround_time = 0.0f;

    // 调度算法名称
    const std::vector<std::string> algorithm_names = {
        u8"先来先服务(FCFS)",
        u8"最短作业优先(SJF)",
        u8"优先级调度",
        u8"时间片轮转(RR)"
    };
};
