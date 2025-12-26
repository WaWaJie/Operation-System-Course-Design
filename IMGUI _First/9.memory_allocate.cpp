#include"9.memory_allocate.h"


//on_enter，进入当前样例的时刻调用，用于初始化样例数据
void MemoryAllocate::on_enter()
{
	str_tip = "";
	str_tip_list.clear();
	process_idx = 1;
	sum_time = 0;
	algo_choose_id = 0;
	current_pcb = nullptr;
	can_execute = false;
	last_allocated_index = 0;
	divide_size= 4;
	begin_addr = 0;
	is_bs = false;
	block_map.clear();
	block_map_bs.clear();
	pcb_memory_map.clear();
	addr_memory_map.clear();
}

//on_exit，离开当前样例的时刻调用，用于清理样例数据，释放内存
void MemoryAllocate::on_exit()
{
	ConfigManager::instance()->clear_resource();
	pcb_queue_normal.clear();
	memory_block_list.clear();
}

//on_update，每帧调用，用于更新样例逻辑和渲染ImGui界面
void MemoryAllocate::on_update(float delta)
{
	on_update_imgui_region1(delta);
	on_update_imgui_region2(delta);
	on_update_imgui_region3(delta);
	if (!can_execute) return;
	sum_time += delta;
	on_update_process(delta);

	// 定期清理无效映射（每5秒一次）
	static float cleanup_timer = 0;
	cleanup_timer += delta;
	if (cleanup_timer >= 5.0f)
	{
		cleanup_invalid_mappings();
		cleanup_timer = 0;
	}
}

//on_update_imgui_region1，更新样例的第一区域ImGui界面
void MemoryAllocate::on_update_imgui_region1(float delta)
{
	ImGui::BeginChild("process_communication_region1", { ImGui::GetContentRegionAvail().x,120 });

	ImGui::Text(u8"算法选择"); ImGui::Separator();
	ImGui::RadioButton(u8"首次适应算法(FF)", &algo_choose_id, 0); ImGui::SameLine();
	ImGui::RadioButton(u8"循环首次适应算法(NF)", &algo_choose_id, 1); ImGui::SameLine();
	ImGui::RadioButton(u8"最佳适应算法(BF)", &algo_choose_id, 2); ImGui::SameLine();
	ImGui::RadioButton(u8"最差适应算法(WF)", &algo_choose_id, 3);
	ImGui::RadioButton(u8"快速适应算法(QF)", &algo_choose_id, 4); ImGui::SameLine();
	ImGui::RadioButton(u8"伙伴系统(BS)", &algo_choose_id, 5);
	ImGui::Separator();

	ImGui::Text(u8"进程信息"); ImGui::Separator();
	ImGui::Text(u8"大小size(KB): "); ImGui::SameLine();
	static int pcb_size = 8;
	ImGui::SetNextItemWidth(200);
	ImGui::SliderInt("##pcb_size", &pcb_size, 1, 32, "%d"); ImGui::SameLine();
	
	if (ImGui::Button(u8"创建进程", { 100,30 }))
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
			pcb->pname = u8"Process" + std::to_string(process_idx++);
			pcb->size = pcb_size;
			pcb->update_state(ProcessState::ActiveReady);

			pcb->timer_time_tick.set_one_shot(false);
			pcb->timer_execute.set_wait_time(2.0);
			pcb->timer_execute.set_one_shot(true);

			auto sp = pcb;
			pcb->timer_execute.set_on_timeout([&, sp]()
				{
					sp->update_state(ProcessState::Finish);
					std::string tip = std::to_string(sum_time) + u8"---进程结束:  " + sp->pname;
					str_tip_list.push_back(TextString(tip, TextType::Info));
				});

			std::string tip = std::to_string(sum_time) + u8"---进程创建:  " + pcb->pname;
			str_tip_list.push_back(TextString(tip, TextType::Info));
			pcb_queue_normal.push_back(pcb);
		}
	}
	
	ImGui::Separator();
	if (algo_choose_id != 5)
	{
		ImGui::Text(u8"内存块信息"); ImGui::Separator();
		ImGui::Text(u8"大小size(KB): "); ImGui::SameLine();
		static int block_size = 8;
		ImGui::SetNextItemWidth(200);
		ImGui::SliderInt("##block_size", &block_size, 1, 32, "%d"); ImGui::SameLine();
		if (ImGui::Button(u8"创建内存块", { 100,30 }))
		{
			auto memory_block= std::make_shared<MemoryBlock>(begin_addr, block_size, true);
			begin_addr += block_size;
			memory_block->bname = "Block" + std::to_string(memory_block_list.size() + 1);
            memory_block_list.push_back(memory_block);
			//快速适应算法下的索引表
			block_map[block_size].push_back(memory_block);

			std::string tip = std::to_string(sum_time) + u8"---内存块创建:  " + memory_block->bname
				+"  " + std::to_string(memory_block->size)+"KB\n";
            str_tip_list.push_back(TextString(tip, TextType::Info));
		}
		ImGui::Text(u8"分割大小(KB): "); ImGui::SameLine();
  		ImGui::SetNextItemWidth(200);
		ImGui::SliderInt("##divide_size", &divide_size, 1, 16, "%d");

		ImGui::Separator();
	}
	else if(!is_bs)
	{
		memory_block_list.clear();
		block_map.clear();
		is_bs = true;
		auto memory_block = std::make_shared<MemoryBlock>(0, 64, true);
        memory_block->bname = "Block" + std::to_string(memory_block_list.size() + 1);
		memory_block_list.push_back(memory_block);
		begin_addr = 64;
		block_map_bs[6].push_back(memory_block);
	}

	ImGui::EndChild();
	ImGui::Separator();
}

//on_update_imgui_region2，更新样例的第二区域ImGui界面
void MemoryAllocate::on_update_imgui_region2(float delta)
{
	ImGui::BeginChild("process_communication_region2", { (float)0.6 * ImGui::GetContentRegionAvail().x,400 });
	ImGui::Text(u8"进程管理"); ImGui::Separator();

	ImGui::BeginChild("memory_allocate_process_list", { 0,0 }, true);
	static const float height_delta = 44;
	for (int i = 0; i < pcb_queue_normal.size(); i++)
	{
		auto& pcb = pcb_queue_normal[i];

		std::stringstream ss;
		ss << std::fixed << std::setprecision(1) << pcb->time_execute;

		std::string label = pcb_queue_normal[i]->pname + ": " + pcb_queue_normal[i]->state_name
			+ u8"--处理时间:" + ss.str() + u8"s--大小:" + std::to_string(pcb_queue_normal[i]->size) + u8"KB";

		// 为每个按钮设置唯一ID
		ImGui::PushID(("allocate_" + std::to_string(i)).c_str());
		if (ImGui::Button(label.c_str(), { 600 * 0.5,40 }))
		{
			current_pcb = pcb_queue_normal[i].get();
			allocate_memory();
		}
		ImGui::PopID();

		ImGui::SameLine();

		// 为结束按钮也设置唯一ID
		// 修改结束按钮的处理逻辑
		// 修改结束按钮的处理逻辑
		ImGui::PushID(("end_" + std::to_string(i)).c_str());
		if (ImGui::Button(u8"结束", { 50,40 }))
		{
			current_pcb = pcb_queue_normal[i].get();

			// 检查进程状态
			if (current_pcb->current_state == ProcessState::Finish)
			{
				std::string tip = std::to_string(sum_time) + u8"---进程结束失败:  进程已经结束 " + current_pcb->pname;
				str_tip_list.push_back(TextString(tip, TextType::Error));
				ImGui::PopID();
				continue;
			}

			// 标记内存块需要回收
			bool found_memory_block = false;
			for (auto& [pcb, block] : pcb_memory_map)
			{
				if (pcb == current_pcb && block)
				{
					block->need_recycle = true;
					found_memory_block = true;
					break;
				}
			}

			if (found_memory_block)
			{
				// 更新进程状态
				current_pcb->update_state(ProcessState::Finish);
				std::string tip = std::to_string(sum_time) + u8"---进程结束:  " + current_pcb->pname;
				str_tip_list.push_back(TextString(tip, TextType::Info));

				// 立即回收内存
				memory_recycle();
			}
			else
			{
				// 没有分配内存的进程也可以结束
				current_pcb->update_state(ProcessState::Finish);
				std::string tip = std::to_string(sum_time) + u8"---进程结束:  " + current_pcb->pname
					+ u8" (未分配内存)";
				str_tip_list.push_back(TextString(tip, TextType::Info));
			}
		}
		ImGui::PopID();

		pcb_queue_normal[i]->draw_state({ 850 * 0.65 + 45,162 + 32 + height_delta * i }, 10);
	}
	ImGui::EndChild(); ImGui::EndChild();ImGui::SameLine();

	ImGui::BeginChild("process_communication_region3", { ImGui::GetContentRegionAvail().x,400 }, true);
	ImGui::Text(u8"内存状态"); ImGui::Separator();

	for (int i = 0; i < memory_block_list.size(); i++)
	{
		auto& block = memory_block_list[i];
		std::string button_name = block->bname + "--> " + block->pname + ":  " + std::to_string(block->size);

		// 主按钮
		ImGui::Button(button_name.c_str(), { 300 * 0.6, 40 });

		ImGui::SameLine(); // 让状态按钮和主按钮在同一行

		std::string button_addr_info = std::to_string(block->start_address) + u8"~" + std::to_string(block->start_address + block->size - 1);

		if (block->is_free)
		{
			// 空闲状态 - 绿色按钮
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.0f, 1.0f)); // 绿色
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));

			ImGui::Button(button_addr_info.c_str(), { 60, 40 });
			ImGui::PopStyleColor(3);
		}
		else
		{
			// 占用状态 - 红色按钮
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // 红色
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

			ImGui::Button(button_addr_info.c_str(), { 60, 40 });
			ImGui::PopStyleColor(3);
		}

		ImGui::Spacing();
	}


	ImGui::EndChild();
	ImGui::Separator();
}

//on_update_imgui_region3，更新样例的第三区域ImGui界面
void MemoryAllocate::on_update_imgui_region3(float delta)
{
	// 信息显示区域
	ImGui::BeginChild("deadlock_check_region4", { ImGui::GetContentRegionAvail().x, 0 }, true);

	ImGui::Button(u8"清空日志", { ImGui::GetContentRegionAvail().x, 30 });
	if (ImGui::IsItemClicked())
	{
		str_tip_list.clear();
	}
	ImGui::Separator();

	// 显示str_tip_list中的所有提示
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
}

//on_update_process，更新样例的进程逻辑	，实现对应的算法逻辑
void MemoryAllocate::on_update_process(float delta)
{
	switch (algo_choose_id)
	{
	case 0:
		break;
	case 1:
		break;
	default:
		break;
	}
}

void MemoryAllocate::allocate_memory()
{
	if (!current_pcb)
	{
		std::string tip = std::to_string(sum_time) + u8"---内存分配:  未选择进程\n";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		return;
	}
	else if(current_pcb->current_state == ProcessState::Finish)
	{
		std::string tip = std::to_string(sum_time) + u8"---内存分配:  进程已结束 " + current_pcb->pname + "\n";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		return;
	}
	else if(current_pcb->current_state == ProcessState::Execute)
	{
		std::string tip = std::to_string(sum_time) + u8"---内存分配:  进程正在执行 " + current_pcb->pname + "\n";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		return;
	}

	switch (algo_choose_id)
	{
	case 0:
		allocate_ff();
		break;
	case 1:
		allocate_nf();
		break;
	case 2:
		allocate_bf();
		break;
	case 3:
        allocate_wf();
        break;
	case 4:
		allocate_qf();
		break;
	case 5:
		allocate_bs();
		break;
	}
}
//首次适应算法
void MemoryAllocate::allocate_ff()
{
	for(int i=0;i<memory_block_list.size();i++)
	{
		auto& block = memory_block_list[i];
		if (block->is_free && block->size >= current_pcb->size)
		{
			// 分配内存块给进程
			block->is_free = false;
			block->pname = current_pcb->pname;
			block->need_recycle = true;
			current_pcb->update_state(ProcessState::Execute);
			std::string tip = std::to_string(sum_time) + u8"---内存分配(FF):  " + current_pcb->pname
				+ u8"  分配到内存块: " + block->bname + "  " + std::to_string(block->size) + "KB\n";
			str_tip_list.push_back(TextString(tip, TextType::Info));
			if (block->size - current_pcb->size >= divide_size)
			{
                // 分割内存块
				auto new_block = std::make_shared<MemoryBlock>();
				new_block->start_address = block->start_address + current_pcb->size;
				new_block->size = block->size - current_pcb->size;
				new_block->is_free = true;
				new_block->bname = "Block" + std::to_string(memory_block_list.size() + 1);
				block->size = current_pcb->size;
				// 在当前块后插入新块
				memory_block_list.insert(memory_block_list.begin() + i + 1, new_block);
				std::string tip_split = std::to_string(sum_time) + u8"---内存块分割(FF):  " + new_block->bname
					+ "  " + std::to_string(new_block->size) + "KB\n";
				str_tip_list.push_back(TextString(tip_split, TextType::Info));
			}
			pcb_memory_map[current_pcb] = block;
			return;
		}
	}
   	std::string tip = std::to_string(sum_time) + u8"---内存分配(FF):  " + current_pcb->pname
           		+ u8"  分配失败\n";
	str_tip_list.push_back(TextString(tip, TextType::Error));
}
//循环首次适应算法
void MemoryAllocate::allocate_nf()
{
	int mx_count = memory_block_list.size();
	int count = 0;
	while (count < mx_count)
	{
		auto&cur_memory_block= memory_block_list[last_allocated_index];
		if(cur_memory_block->is_free && cur_memory_block->size >= current_pcb->size)
		{
			// 分配内存块给进程
			cur_memory_block->is_free = false;
			cur_memory_block->pname = current_pcb->pname;
			current_pcb->update_state(ProcessState::Execute);
			std::string tip = std::to_string(sum_time) + u8"---内存分配(NF):  " + current_pcb->pname
				+ u8"  分配到内存块: " + cur_memory_block->bname + "  " + std::to_string(cur_memory_block->size) + "KB\n";
			str_tip_list.push_back(TextString(tip, TextType::Info));

			if (cur_memory_block->size - current_pcb->size >= divide_size)
			{
				// 分割内存块
				auto new_block = std::make_shared<MemoryBlock>();
				new_block->start_address = cur_memory_block->start_address + current_pcb->size;
				new_block->size = cur_memory_block->size - current_pcb->size;
				new_block->need_recycle = true;
				new_block->bname = "Block" + std::to_string(memory_block_list.size() + 1);
				cur_memory_block->size = current_pcb->size;
				// 在当前块后插入新块
				memory_block_list.insert(memory_block_list.begin() + last_allocated_index + 1, new_block);
				std::string tip_split = std::to_string(sum_time) + u8"---内存块分割(NF):  " + new_block->bname
					+ "  " + std::to_string(new_block->size) + "KB\n";
				str_tip_list.push_back(TextString(tip_split, TextType::Info));
			}
			pcb_memory_map[current_pcb] = cur_memory_block;
			return;
		}
		last_allocated_index = (last_allocated_index + 1) % memory_block_list.size();
		count++;
	}
    std::string tip = std::to_string(sum_time) + u8"---内存分配(NF):  " + current_pcb->pname
		+ u8"  分配失败\n";
	str_tip_list.push_back(TextString(tip, TextType::Error));

}
//最佳适应算法
void MemoryAllocate::allocate_bf()
{
	std::sort(memory_block_list.begin(), memory_block_list.end(),
        		[](const std::shared_ptr<MemoryBlock>& a, const std::shared_ptr<MemoryBlock>& b) {
			return a->size < b->size;
		});
	allocate_ff();
}
//最坏适应算法
void MemoryAllocate::allocate_wf()
{
    std::sort(memory_block_list.begin(), memory_block_list.end(),
				[](const std::shared_ptr<MemoryBlock>& a, const std::shared_ptr<MemoryBlock>& b) {
		return a->size > b->size;
	});
	allocate_ff();
}
//快速适应算法
void MemoryAllocate::allocate_qf()
{
	for (auto& table_entry : block_map)
	{
		if (table_entry.first >= current_pcb->size && !table_entry.second.empty())
		{
			auto block = table_entry.second.front();
			table_entry.second.pop_front();
			// 分配内存块给进程
			block->is_free = false;
			block->pname = current_pcb->pname;
			current_pcb->update_state(ProcessState::Execute);
			std::string tip = std::to_string(sum_time) + u8"---内存分配(QF):  " + current_pcb->
				pname + u8"  分配到内存块: " + block->bname + "  " + std::to_string(block->size) + "KB\n";
			str_tip_list.push_back(TextString(tip, TextType::Info));
			pcb_memory_map[current_pcb] = block;
			return;
		}
	}
    std::string tip = std::to_string(sum_time) + u8"---内存分配(QF):  " + current_pcb->pname
		+ u8"  分配失败\n";
    str_tip_list.push_back(TextString(tip, TextType::Error));
}
//伙伴系统
void MemoryAllocate::allocate_bs()
{
	int size_to_allocate = current_pcb->size;
	int k_need = 0;
	for (int i = 0; i < 6; i++)
	{
		if(pow(2, i) >= size_to_allocate&&pow(2, i) < pow(2, i + 1))
   		{
			k_need = i;
			break;
		}
	}
	std::string k_tip = std::to_string(sum_time) + u8"---内存分配(BS):  " + current_pcb->pname
        		+ u8"  需要分配的内存块大小为: " + std::to_string(size_to_allocate) + "KB"
        		+ u8"  需要分配的k值为: " + std::to_string(k_need) + "\n";
	str_tip_list.push_back(TextString(k_tip, TextType::KeyInfo));
	
	for (auto& table_entry : block_map_bs)
	{
		if (table_entry.first == k_need && !table_entry.second.empty())
		{
			auto block = table_entry.second.front();
			table_entry.second.pop_front();
			// 分配内存块给进程
			block->is_free = false;
			block->pname = current_pcb->pname;
			current_pcb->update_state(ProcessState::Execute);
			pcb_memory_map[current_pcb] = block;

			std::string tip = std::to_string(sum_time) + u8"---内存分配(BS):  " + current_pcb->
				pname + u8"  分配到内存块: " + block->bname + "  " + std::to_string(block
					->size) + "KB\n";
			str_tip_list.push_back(TextString(tip, TextType::Info));
			return;
		}
        else if (table_entry.first > k_need && !table_entry.second.empty())
		{
			// 找到一个更大的块，进行分割
			auto block = table_entry.second.front();
			table_entry.second.pop_front();
			while (block->size / 2 >= size_to_allocate)
			{
				// 分割块
				auto buddy_block = std::make_shared<MemoryBlock>();
				buddy_block->start_address = block->start_address + block->size / 2;
				buddy_block->size = block->size / 2;
				buddy_block->is_free = true;
				buddy_block->bname = "Block" + std::to_string(memory_block_list.size() + 1);
				memory_block_list.push_back(buddy_block);
				block->size /= 2;
				// 更新映射表
				block_map_bs[get_k(buddy_block->size)].push_back(buddy_block);
				addr_memory_map[buddy_block->start_address] = buddy_block;
			}
			// 分配内存块给进程
			block->is_free = false;
			block->pname = current_pcb->pname;
			pcb_memory_map[current_pcb] = block;
			addr_memory_map[block->start_address] = block;
			current_pcb->update_state(ProcessState::Execute);
			std::string tip = std::to_string(sum_time) + u8"---内存分配(BS):  " + current_pcb->
				pname + u8"  分配到内存块: " + block->bname + "  " + std::to_string(block
					->size) + "KB\n";
			str_tip_list.push_back(TextString(tip, TextType::Info));
			return;
		}
	}
	std::string tip = std::to_string(sum_time) + u8"---内存分配(BS):  " + current_pcb->pname
		+ u8"  分配失败\n";
	str_tip_list.push_back(TextString(tip, TextType::Error));
}

void MemoryAllocate::memory_recycle()
{
	switch (algo_choose_id)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		memory_recycle_line();
		break;
	case 4:
		memory_recycle_qf();
		break;
	case 5:
		memory_recycle_bs();
		break;
	default:
		std::string tip = std::to_string(sum_time) + u8"---内存回收:  " + current_pcb->pname
            			+ u8"  分配失败\n"+"\t\t\t\t原因: 未选择算法\n";
        str_tip_list.push_back(TextString(tip, TextType::Error));
        break;
	}
}

void MemoryAllocate::memory_recycle_line()
{
	// 先找到需要回收的块
	for (int i = 0; i < memory_block_list.size(); i++)
	{
		auto block = memory_block_list[i];

		// 只回收当前进程的块
		if (block->need_recycle)
		{
			// 从PCB映射中找到对应的PCB
			PCB* matching_pcb = nullptr;
			for (const auto& [pcb, mem_block] : pcb_memory_map)
			{
				if (mem_block == block && pcb == current_pcb)
				{
					matching_pcb = pcb;
					break;
				}
			}

			if (!matching_pcb)
			{
				// 可能已经回收过了
				continue;
			}

			// 打印回收信息
			std::string tip = std::to_string(sum_time) + u8"---内存回收:  开始回收 " + block->bname
				+ u8" 大小: " + std::to_string(block->size) + "KB\n";
			str_tip_list.push_back(TextString(tip, TextType::Info));

			// 设置块为空闲
			block->is_free = true;
			block->need_recycle = false;
			block->pname = "";

			// 从PCB映射中移除
			pcb_memory_map.erase(matching_pcb);

			// 检查是否能与前一块合并
			if (i > 0 && memory_block_list[i - 1]->is_free)
			{
				auto prev_block = memory_block_list[i - 1];

				tip = std::to_string(sum_time) + u8"---内存回收:  向前合并 "
					+ block->bname + u8"(" + std::to_string(block->size) + u8"KB) 到 "
					+ prev_block->bname + u8"(" + std::to_string(prev_block->size) + u8"KB)\n";
				str_tip_list.push_back(TextString(tip, TextType::Info));

				// 合并到前一块
				prev_block->size += block->size;

				// 删除当前块
				memory_block_list.erase(memory_block_list.begin() + i);

				// 更新i，因为列表改变了
				i--;

				// 更新block为合并后的块
				block = prev_block;
			}

			// 检查是否能与后一块合并
			if (i < memory_block_list.size() - 1 && memory_block_list[i + 1]->is_free)
			{
				auto next_block = memory_block_list[i + 1];

				tip = std::to_string(sum_time) + u8"---内存回收:  向后合并 "
					+ next_block->bname + u8"(" + std::to_string(next_block->size) + u8"KB) 到 "
					+ block->bname + u8"(" + std::to_string(block->size) + u8"KB)\n";
				str_tip_list.push_back(TextString(tip, TextType::Info));

				// 合并到当前块
				block->size += next_block->size;

				// 删除后一块
				memory_block_list.erase(memory_block_list.begin() + i + 1);

				// 注意：这里不更新i，因为我们只需要合并一次
			}

			tip = std::to_string(sum_time) + u8"---内存回收:  回收完成，最终块 "
				+ block->bname + u8" 大小: " + std::to_string(block->size) + "KB\n";
			str_tip_list.push_back(TextString(tip, TextType::KeyInfo));

			return; // 只回收一个块就返回
		}
	}

	// 如果没有找到需要回收的块
	std::string tip = std::to_string(sum_time) + u8"---内存回收:  未找到需要回收的内存块\n";
	str_tip_list.push_back(TextString(tip, TextType::Warning));
}

void MemoryAllocate::memory_recycle_qf()
{
	for (int i = 0; i < memory_block_list.size(); i++)
	{
		auto& block = memory_block_list[i]; // 复制 shared_ptr（不再是引用）
		if (block->need_recycle)
		{
			block->is_free = true;
			block->need_recycle = false;
			
			block_map[block->size].push_back(block);

			std::string tip = std::to_string(sum_time) + u8"---内存回收:  "
				+ u8"  回收成功\n" + u8"\t\t\t\t回收的内存块为: " + block->bname + "  " + std::to_string(block->size)
				+ "KB\n";
			str_tip_list.push_back(TextString(tip, TextType::Info));

			break;
		}
	}
}

void MemoryAllocate::memory_recycle_bs()
{
	if (!current_pcb)
	{
		std::string tip = std::to_string(sum_time) + u8"---内存回收(BS):  未选择进程\n";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		return;
	}

	auto it = pcb_memory_map.find(current_pcb);
	if (it == pcb_memory_map.end())
	{
		std::string tip = std::to_string(sum_time) + u8"---内存回收(BS):  进程未分配内存 " + current_pcb->pname + "\n";
		str_tip_list.push_back(TextString(tip, TextType::Error));
		return;
	}

	// 获取要回收的块
	auto block = it->second;
	block->is_free = true;
	block->pname = "Empty";
	block->need_recycle = false;

	std::string tip = std::to_string(sum_time) + u8"---内存回收(BS):  开始回收 " + block->bname
		+ u8" 大小: " + std::to_string(block->size) + "KB\n";
	str_tip_list.push_back(TextString(tip, TextType::Info));

	// 递归合并伙伴块
	bool merged = true;
	while (merged && block->size < 64) // 最大块为64KB
	{
		merged = false;

		// 计算伙伴地址
		int cur_k = get_k(block->size);
		int buddy_addr = get_buddy_addr(block->start_address, cur_k);

		// 查找伙伴块
		auto buddy_it = addr_memory_map.find(buddy_addr);
		if (buddy_it != addr_memory_map.end())
		{
			auto buddy_block = buddy_it->second;

			// 检查伙伴块是否空闲且大小相同
			if (buddy_block->is_free && buddy_block->size == block->size)
			{
				tip = std::to_string(sum_time) + u8"---内存回收(BS):  找到空闲伙伴块 "
					+ buddy_block->bname + u8" 大小: " + std::to_string(buddy_block->size) + "KB\n";
				str_tip_list.push_back(TextString(tip, TextType::Info));

				// 合并两个块
				// 保留起始地址小的块
				std::shared_ptr<MemoryBlock> merged_block;
				std::shared_ptr<MemoryBlock> block_to_remove;

				if (block->start_address < buddy_block->start_address)
				{
					merged_block = block;
					block_to_remove = buddy_block;
				}
				else
				{
					merged_block = buddy_block;
					block_to_remove = block;
				}

				// 更新合并后的块
				merged_block->size *= 2;
				merged_block->bname = "Block" + std::to_string(std::rand() % 1000); // 生成新名字

				tip = std::to_string(sum_time) + u8"---内存回收(BS):  合并块 " + block->bname
					+ u8" 和 " + buddy_block->bname + u8" 新块: " + merged_block->bname
					+ u8" 大小: " + std::to_string(merged_block->size) + "KB\n";
				str_tip_list.push_back(TextString(tip, TextType::KeyInfo));

				// 从列表中删除被合并的块
				auto remove_it = std::remove_if(memory_block_list.begin(), memory_block_list.end(),
					[&block_to_remove](const std::shared_ptr<MemoryBlock>& b) {
						return b.get() == block_to_remove.get();
					});
				memory_block_list.erase(remove_it, memory_block_list.end());

				// 从地址映射中删除被合并的块
				addr_memory_map.erase(block_to_remove->start_address);

				// 从块大小映射中删除两个旧块，添加新块
				int old_k = cur_k;
				int new_k = get_k(merged_block->size);

				// 从旧的映射中移除
				auto& old_list = block_map_bs[old_k];
				old_list.remove_if([&](const std::shared_ptr<MemoryBlock>& b) {
					return b.get() == block.get() || b.get() == buddy_block.get();
					});

				// 如果旧列表为空，删除该条目
				if (old_list.empty())
				{
					block_map_bs.erase(old_k);
				}

				// 添加到新的映射
				block_map_bs[new_k].push_back(merged_block);

				// 更新地址映射
				addr_memory_map[merged_block->start_address] = merged_block;

				// 继续尝试合并
				block = merged_block;
				merged = true;
			}
		}
	}

	// 最终将块添加到空闲列表
	int k = get_k(block->size);
	block_map_bs[k].push_back(block);
	addr_memory_map[block->start_address] = block;

	// 从PCB映射中移除
	pcb_memory_map.erase(it);

	tip = std::to_string(sum_time) + u8"---内存回收(BS):  回收完成 " + block->bname
		+ u8" 最终大小: " + std::to_string(block->size) + "KB\n";
	str_tip_list.push_back(TextString(tip, TextType::KeyInfo));
}

void MemoryAllocate::cleanup_invalid_mappings()
{
	std::vector<PCB*> to_remove;

	for (auto& [pcb, block] : pcb_memory_map)
	{
		// 检查PCB是否有效
		bool pcb_found = false;
		for (auto& p : pcb_queue_normal)
		{
			if (p.get() == pcb)
			{
				pcb_found = true;
				break;
			}
		}

		if (!pcb_found || !block)
		{
			to_remove.push_back(pcb);
		}
	}

	for (auto pcb : to_remove)
	{
		pcb_memory_map.erase(pcb);
	}

	if (!to_remove.empty())
	{
		std::string tip = std::to_string(sum_time) + u8"---清理了 " + std::to_string(to_remove.size())
			+ u8" 个无效的内存映射\n";
		str_tip_list.push_back(TextString(tip, TextType::Warning));
	}
}


//recycle