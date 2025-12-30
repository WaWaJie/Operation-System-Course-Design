#include<string>
#include <memory>
#include "process_control_block.h"

// 在原有基础上添加线程相关状态
enum class ThreadState
{
    Running,      // 运行
    Ready,        // 就绪
    Blocked,      // 阻塞
    Terminated    // 终止
};


// 线程控制块结构
struct TCB {
    int tid;                    // 线程ID
    std::string tname;          // 线程名称
    ThreadState state;          // 线程状态
    float time_needed;          // 需要执行的总时间
    float time_executed;        // 已执行的时间
    float time_remaining;       // 剩余执行时间
    int priority;               // 优先级 (1-3，1最高)
    std::shared_ptr<PCB> parent_process; // 所属进程
    float wait_time;            // 等待时间
    float turnaround_time;      // 周转时间

    TCB(int id, std::string name, std::shared_ptr<PCB> parent)
        : tid(id), tname(name), state(ThreadState::Ready),
        time_needed(1.5f + static_cast<float>(rand() % 100) / 100.0f), // 1.5-2.5秒
        time_executed(0.0f), time_remaining(0.0f),
        priority(1 + rand() % 3), parent_process(parent),
        wait_time(0.0f), turnaround_time(0.0f)
    {
        time_remaining = time_needed;
    }
};