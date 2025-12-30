#pragma once

#include"almighty_head.h"

class MemoryAllocate :public Example
{
public:

	void on_enter() override;
	void on_update(float delta)override;
	void on_exit() override;

private:

	void on_update_imgui_region1(float delta);
	void on_update_imgui_region2(float delta);
	void on_update_imgui_region3(float delta);

	void on_update_process(float delta);

	//可以使用策略模式&&桥接模式进行优化
	void allocate_memory();
	void allocate_ff();
	void allocate_nf();
	void allocate_bf();
	void allocate_wf();
	void allocate_qf();
	void allocate_bs();

	void memory_recycle();
	void memory_recycle_line();
	void memory_recycle_qf();
	void memory_recycle_bs();
	void cleanup_invalid_mappings();

	int get_k(int val)
	{
		int res = 0;
		while (val != 1)
		{
			res++;
			val >>= 1;
		}
		return res;
	}

	int qpow(int x, int y)
	{
		int res = 1;
		while (y)
		{
			if (y & 1)res *= x;
			x *= x;
			y >>= 1;
		}
		return res;
	}

	int get_buddy_addr(int x, int k)
	{
		if (x % qpow(2, k+1) == 0)
			return x + qpow(2, k);
		else
			return x - qpow(2, k);
	};
	
private:

private:
	float sum_time = 0;			//程序自运行开始的总时间
	int process_idx = 1;		//进程索引
	bool can_execute = false;	//是否可以执行

	int algo_choose_id = 0;		//算法选择id

	float process_time = 1.0f;	//进程处理时间

	PCB* current_pcb = nullptr;		//当前选中的进程控制块
	std::vector<std::shared_ptr<PCB>>pcb_queue_normal;	//进程控制块队列
	std::vector<std::shared_ptr<MemoryBlock>>memory_block_list;	//内存块列表
	int begin_addr = 0;
	int last_allocated_index = 0;//用于nf算法，记录上次分配的内存块索引
	int divide_size = 4;		//内存块分割大小
	bool is_bs = false;			//是否是伙伴系统算法
	
	std::map<int, std::list<std::shared_ptr<MemoryBlock>>>block_map;	//内存块映射表
	std::map<int, std::list<std::shared_ptr<MemoryBlock>>>block_map_bs;	//伙伴系统的内存块映射表
	std::unordered_map<PCB*,std::shared_ptr<MemoryBlock>> pcb_memory_map;	//进程与内存块映射表
	std::unordered_map<int, std::shared_ptr<MemoryBlock>>addr_memory_map;	//内存地址与内存块映射表
	
	std::string str_tip = "";	//运行提示信息
	std::vector<TextString>str_tip_list;
};