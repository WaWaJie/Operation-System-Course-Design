#pragma once

#include"timer.h"
#include"process_state.h"
#include"config_manager.h"
#include"memory_block.h"
#include<string>
#include<unordered_map>


struct Message
{
    std::string id = "";
    std::string from = "";
    std::string to = "";
    std::string content = "";
    bool is_received = false;
};


struct PCB
{
    bool can_execute = false;

    std::string pname;      //进程名称
    std::string state_name; //进程状态名称

    ProcessState current_state = ProcessState::StaticBlock;
    Timer timer_execute;    //进程处理时间，默认3s
    Timer timer_time_tick;  //时间片，默认1s  
    Timer timer_deadlock_prevention_execute; //预防死锁时间，默认1s

    bool can_wait = true;
    Timer timer_to_wait_max;   //资源等待最长时间，默认1s
    Timer timer_to_ask_for_resource; //请求资源时间间隔，默认0.1s

    //用于进程调度
    float time_execute = 0.0f;//处理时间
    float ready_time = 0.0f;	//就绪时间
    float deadline_start = 0.0f;  //开始截止时间
    float deadline_end = 0.0f;    //结束截止时间
    int priority = 1;   //优先级  
    int fair_ratio = 1; //公平占比 

    std::unordered_map<std::string, int>resource_hold; //持有的资源
    std::unordered_map<std::string, int>resource_max;  //需要的资源
    std::unordered_map<std::string, int>resource_to_request; //请求的资源
    int last_hold_num[4] = { 0 };
	bool is_released_all_resource = false; //是否已经释放所有资源
	bool show_resource_window = false; //是否显示资源信息窗口
    std::string resource_occupied_name;                 //占有资源的名称

    std::vector<std::shared_ptr<Message>>msg_list;   //信息列表（信息由其他进程发送而来）
    std::vector<std::shared_ptr<Message>>msg_list_received; //已经获取的信息列表

    std::vector<std::shared_ptr<ProcessResource>>task_list; //任务列表

    //所需内存块大小
    int size = 0;
    int start_addr = 0;

    //更新进程状态
    void update_state(ProcessState state)
    {
        current_state = state;
        switch (state)
        {
        case ProcessState::ActiveBlock:
            state_name = "活动阻塞";
            break;
        case ProcessState::ActiveReady:
            state_name = "活动就绪";
            break;
        case ProcessState::Execute:
            state_name = "执行";
            break;
        case ProcessState::Finish:
            state_name = "终止";
            break;
        case ProcessState::StaticBlock:
            state_name = "静止阻塞";
            break;
        case ProcessState::StaticReady:
            state_name = "静止就绪";
            break;
        }
    }
    //绘制进程状态  
    void draw_state(ImVec2 pos, int r)
    {
        ImColor color = { 0,0,0 };
        switch (current_state)
        {
        case ProcessState::ActiveBlock:
            color = ConfigManager::instance()->get_color_pool().find(ProcessState::ActiveBlock)->second;
            break;
        case ProcessState::ActiveReady:
            color = ConfigManager::instance()->get_color_pool().find(ProcessState::ActiveReady)->second;
            break;
        case ProcessState::Execute:
            color = ConfigManager::instance()->get_color_pool().find(ProcessState::Execute)->second;
            break;
        case ProcessState::Finish:
            color = ConfigManager::instance()->get_color_pool().find(ProcessState::Finish)->second;
            break;
        case ProcessState::StaticBlock:
            color = ConfigManager::instance()->get_color_pool().find(ProcessState::StaticBlock)->second;
            break;
        case ProcessState::StaticReady:
            color = ConfigManager::instance()->get_color_pool().find(ProcessState::StaticReady)->second;
            break;
        }
        ImGui::GetWindowDrawList()->AddCircleFilled(pos, r, color);
    }


    bool is_resources_ok()
    {
        for (auto& resource : resource_max)
        {
            if (resource_hold[resource.first] < resource.second)
                return false;
        }
        return true;
    }

    void wait_record(const std::string& resource_name)
    {
        if (ConfigManager::instance()->get_resource_number(resource_name))
        {
            int add_number_final = std::min(resource_max[resource_name] - resource_hold[resource_name], ConfigManager::instance()->get_resource_number(resource_name));
            resource_hold[resource_name] += add_number_final;
            ConfigManager::instance()->sub_resource(resource_name, add_number_final);
        }
    }

    void signal_record(const std::string& resource_name)
    {
        ConfigManager::instance()->add_resource(resource_name, resource_hold[resource_name]);
        resource_hold[resource_name] = 0;
    }

    void wait_and(const std::vector<std::string>&resource_name_list)
    {
        bool can_allocate = true;
        for (auto& resource_name : resource_name_list)
        {
            if (ConfigManager::instance()->get_resource_number(resource_name) < (resource_max[resource_name] - resource_hold[resource_name]))
            {
                can_allocate = false;
                break;
            }
        }
        if (can_allocate)
        {
            for (auto& resource_name : resource_name_list)
            {
                int add_number_final = resource_max[resource_name] - resource_hold[resource_name];
                resource_hold[resource_name] += add_number_final;
                ConfigManager::instance()->sub_resource(resource_name, add_number_final);
            }
        }
    }

    void signal_and()
    {
        for (auto& resource : resource_hold)
        {
            ConfigManager::instance()->add_resource(resource.first, resource.second);
            resource.second = 0;
        }
    }

    //构造函数，初始化定时器
    PCB()
    {
        timer_execute.set_wait_time(3.0f);
        timer_execute.set_on_timeout([&]()
            {
                update_state(ProcessState::Finish);
            });
        timer_execute.set_one_shot(true);

        timer_time_tick.set_wait_time(1.0f);
        timer_time_tick.set_on_timeout([&]()
            {
                update_state(ProcessState::ActiveReady);
            });
        timer_time_tick.set_one_shot(false);

        timer_to_wait_max.set_one_shot(false);
        timer_to_wait_max.set_wait_time(1.0f);
        timer_to_wait_max.set_on_timeout([&]()
            {
                can_wait = false;
                timer_to_ask_for_resource.restart();
                update_state(ProcessState::ActiveBlock);
            });
        timer_to_ask_for_resource.set_one_shot(false);
        timer_to_ask_for_resource.set_wait_time(0.1f);
        timer_to_ask_for_resource.set_on_timeout([&]()
            {
                can_wait = true;
                timer_to_wait_max.restart();
            });
        for (int i = 0; i < 4; i++)
            last_hold_num[i] = 0;
    }

};

