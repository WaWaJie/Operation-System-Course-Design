// 10.es_visit.h (更新)
#pragma once

#include"almighty_head.h"

class EsVisit :public Example
{
public:

    void on_enter() override;
    void on_update(float delta)override;
    void on_exit() override;

private:

    void on_update_imgui_region1(float delta);
    void on_update_imgui_region2(float delta);
    void on_update_imgui_region3(float delta);
    void on_update_imgui_region4(float delta);

    void on_update_process(float delta);
    
    // 新添加的方法
    void generate_track_sequence();  // 生成随机访问序列
    void execute_fcfs();              // 执行FCFS算法
    void execute_sstf();              // 执行SSTF算法
    void reset_simulation();          // 重置模拟

private:

    // 算法执行结果结构
    struct AlgorithmResult {
        std::vector<int> sequence;     // 访问顺序
        std::vector<int> distances;    // 移动距离
        float avg_seek_length;         // 平均寻道长度
    };

    // 磁道模拟相关
    int start_pos = 100;               // 磁头起始位置
    int max_track = 200;               // 最大磁道数
    int sequence_size = 10;            // 访问序列大小
    std::vector<int> track_list;       // 访问序列
    
    // 算法结果存储
    AlgorithmResult fcfs_result;
    AlgorithmResult sstf_result;
    
    // 原有成员变量
    float sum_time = 0;                // 程序自运行开始的总时间
    bool can_execute = false;          // 是否可以执行
    int algo_choose_id = 0;            // 算法选择id
    std::vector<TextString>str_tip_list;
};