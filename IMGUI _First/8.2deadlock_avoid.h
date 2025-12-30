#pragma once

#include"almighty_head.h"

class DeadLockAvoid : public Example
{
public:
    void on_enter() override;
    void on_update(float delta) override;
    void on_input(const SDL_Event* event) override;
    void on_exit() override;

private:
    void on_update_imgui_region1(float delta); // 资源管理
    void on_update_imgui_region2(float delta); // 进程管理
    void on_update_imgui_region3(float delta); // 银行家算法控制
    void on_update_imgui_region4(float delta); // 信息显示
    void on_update_process(float delta);

    // 银行家算法相关函数
    bool banker_algorithm_safety_check();
    bool banker_algorithm_resource_request(int process_index, const std::vector<int>& request);
    void allocate_resources_to_process(int process_index);
    void release_resources_from_process(int process_index);
    void update_process_allocation(int process_index);
    void update_process_max_demand(int process_index);


    // UI辅助函数
    void render_resource_matrices();
    void render_process_allocation_window();
    void render_process_max_demand_window();
    void render_resource_request_window();

private:
    float sum_time = 0;
    int process_idx = 1;
    bool can_execute = false;
    bool show_safety_info = false;
    bool show_allocation_window = false;
    bool show_max_demand_window = false;
    bool show_request_window = false;

    PCB* current_pcb = nullptr;
    std::vector<std::shared_ptr<PCB>> pcb_queue_normal;
    std::vector<std::shared_ptr<PCB>> pcb_queue_can_execute;

    // 银行家算法数据结构
    std::vector<std::string> resource_names;
    std::vector<int> available;  // 可用资源向量
    std::vector<std::vector<int>> max_demand;  // 最大需求矩阵
    std::vector<std::vector<int>> allocation;  // 分配矩阵  
    std::vector<std::vector<int>> need;  // 需求矩阵
    std::vector<int> safe_sequence;  // 安全序列

    // UI状态
    std::string str_tip = "";
    std::vector<TextString>str_tip_list;

    int selected_process = 0;
    std::vector<int> request_resources;
    std::vector<int> allocation_edit; // 用于编辑分配资源
    std::vector<int> max_demand_edit; // 用于编辑最大需求

    // 资源管理
    ProcessResource resource_to_add[4];
    int num_of_resource_to_add = 1;
    int selected_resource_type = 0;
};

//Finish