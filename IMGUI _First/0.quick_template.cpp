#include"0.quick_template.h"

/*
说明：这是你快速开发的模板，你可以根据下面的提示进行你的创作。

（1）如果你需要创建新的变量，可到头文件0.quick_template.h中添加成员变量，
	并在on_enter和on_exit中进行初始化和清理工作。
	此外，如果是PCB的信息的话，可以看头文件process_control_block.h中PCB结构体的成员变量，
	你可以对里面的成员变量进行使用和扩展。但是，请不要修改PCB结构体本身，以免影响其他样例的运行。
（2）如果你需要创建新的函数，可到头文件0.quick_template.h中添加成员函数声明，
	并在本文件中进行函数定义。
（3）正常情况下，on_update_imgui_region2和on_update_imgui_region3函数不需要修改，
	你只需要在on_update_imgui_region1函数中实现对应的操作按钮，完成算法的选择，必要资源的获取，对应进程的创建，并且
	在on_update_process函数中实现对应的算法逻辑即可。
*/

//on_enter，进入当前样例的时刻调用，用于初始化样例数据
void QuickTemplate::on_enter()
{
	str_tip = "";
	process_idx = 1;
	sum_time = 0;
	algo_choose_id = 0;
	current_pcb = nullptr;
	can_execute = false;
}

//on_exit，离开当前样例的时刻调用，用于清理样例数据，释放内存
void QuickTemplate::on_exit()
{
	ConfigManager::instance()->clear_resource();
	pcb_queue_normal.clear();
}

//on_update，每帧调用，用于更新样例逻辑和渲染ImGui界面
void QuickTemplate::on_update(float delta)
{
	on_update_imgui_region1(delta);
	on_update_imgui_region2(delta);
	on_update_imgui_region3(delta);
	if (!can_execute)return;
	sum_time += delta;
	on_update_process(delta);
}

//on_update_imgui_region1，更新样例的第一区域ImGui界面
void QuickTemplate::on_update_imgui_region1(float delta)
{
	ImGui::BeginChild("process_communication_region1", { ImGui::GetContentRegionAvail().x,120 });
	/*
	你的创作区域
	*/

	ImGui::EndChild();
	ImGui::Separator();
}

//on_update_imgui_region2，更新样例的第二区域ImGui界面
void QuickTemplate::on_update_imgui_region2(float delta)
{
	ImGui::BeginChild("process_communication_region2", { ImGui::GetContentRegionAvail().x,400 });

	static const float height_delta = 44;
	for (int i = 0; i < pcb_queue_normal.size(); i++)
	{
		auto& pcb = pcb_queue_normal[i];

		std::stringstream ss;
		ss << std::fixed << std::setprecision(1) << pcb->time_execute;

		std::string label = pcb_queue_normal[i]->pname + ": " + pcb_queue_normal[i]->state_name
			+ u8"--处理时间:" + ss.str();

		ImGui::Button(label.c_str(), { 600,40 });
		if (ImGui::IsItemClicked())
		{
			current_pcb = pcb_queue_normal[i].get();
		}

		pcb_queue_normal[i]->draw_state({ 850,162 + height_delta * i }, 10);
	}

	ImGui::EndChild();
	ImGui::Separator();
}

//on_update_imgui_region3，更新样例的第三区域ImGui界面
void QuickTemplate::on_update_imgui_region3(float delta)
{
	ImGui::BeginChildFrame(1, { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y });
	ImGui::Button(u8"清空日志", { ImGui::GetContentRegionAvail().x,30 });
	if (ImGui::IsItemClicked())
	{
		str_tip = "";
	}
	ImGui::TextUnformatted(str_tip.c_str());

	static std::string str_last_tip = str_tip;
	if (str_last_tip != str_tip)
	{
		ImGui::SetScrollY(ImGui::GetScrollMaxY() + 2000);
	}
	str_last_tip = str_tip;

	ImGui::EndChildFrame();
}

//on_update_process，更新样例的进程逻辑	，实现对应的算法逻辑
void QuickTemplate::on_update_process(float delta)
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

