#pragma once

#include "almighty_head.h"

class InAndOut : public Example
{
public:
    void on_enter() override;
    void on_update(float delta) override;
    void on_exit() override;

private:
    void on_update_imgui_region1(float delta);
    void on_update_imgui_region2(float delta);
    void on_update_imgui_region3(float delta);
    void on_update_process(float delta);

    // 算法实现
    void access_page(int page_id);
    void handle_page_fault(int page_id);
    void replace_page_lru();
    void replace_page_lfu();

    // 工具函数
    void generate_access_sequence();
    void reset_statistics();
    void update_statistics(bool hit);
    void next_access();
    void end_simulation();

    // 显示辅助函数
    void draw_memory_table();
    void draw_page_list();
    void draw_statistics();

private:
    // 算法参数
    int algorithm_choice = 0;          // 0:LRU, 1:LFU
    int virtual_memory_size = 16;      // 虚拟内存大小（页面数）
    int physical_memory_size = 4;      // 物理内存大小（页面数）
    int sequence_length = 20;          // 访问序列长度
    int current_time = 0;              // 当前时间（访问次数）
    int access_index = 0;              // 当前访问序列索引

    // 控制参数
    float simulation_speed = 1.0f;     // 模拟速度
    float elapsed_time = 0.0f;         // 累计时间
    bool is_auto_mode = false;         // 自动执行模式
    bool is_running = false;           // 是否正在运行
    bool is_finished = false;          // 是否已结束
    bool show_help = false;            // 显示帮助信息

    // 统计信息
    int hit_count = 0;                 // 命中次数
    int fault_count = 0;               // 缺页次数
    int total_access = 0;              // 总访问次数
    float hit_rate = 0.0f;             // 命中率
    float fault_rate = 0.0f;           // 缺页率

    // 数据结构
    std::vector<int> access_sequence;  // 访问序列
    std::vector<Page> pages;           // 所有页面
    std::list<int> memory_frames;      // 物理内存中的页面（按LRU顺序）
    std::set<int> memory_set;          // 物理内存中的页面集合

    // 显示信息
    std::string current_status;        // 当前状态
    std::string last_operation;        // 最后操作
    std::vector<std::string> logs;     // 日志记录
    std::vector<int> last_replaced;    // 最近被替换的页面

    // UI颜色
    ImVec4 hit_color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);      // 命中-绿色
    ImVec4 fault_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);    // 缺页-红色
    ImVec4 memory_color = ImVec4(0.1f, 0.5f, 0.9f, 1.0f);   // 内存中-蓝色
    ImVec4 normal_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // 正常-白色
};