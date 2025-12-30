#pragma once

#include"almighty_head.h"

class DeadLockCheckAndRelease : public Example
{
public:
    void on_enter() override;
    void on_update(float delta) override;
    void on_input(const SDL_Event* event) override;
    void on_exit() override;

private:
    void on_update_imgui_region1(float delta); // 资源管理
    void on_update_imgui_region2(float delta); // 进程管理
    void on_update_imgui_region3(float delta); // 死锁检测控制
    void on_update_imgui_region4(float delta); // 信息显示

	void on_update_process_resource(float delta); // 进程资源请求与释放

    // 死锁检测函数
    bool detect_deadlock();

    // 死锁解除函数
    void release_all_processes();

    // UI辅助函数
    void render_detection_info();

private:
    float sum_time = 0;
    int process_idx = 1;
    bool can_execute = false;
    bool deadlock_detected = false;

    PCB* current_pcb = nullptr;
    std::vector<std::shared_ptr<PCB>> pcb_queue_normal;

    // 死锁检测数据结构
    std::vector<std::string> resource_names;
    std::vector<int> available;
    std::vector<std::vector<int>> allocation;
    std::vector<std::vector<int>> request;

    // UI状态
    std::vector<TextString> str_tip_list;

    // 资源管理
    int selected_resource_type = 0;
    int num_of_resource_to_add = 1;

};