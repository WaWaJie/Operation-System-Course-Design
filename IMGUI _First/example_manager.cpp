#include"example_manager.h"
#include"resources_manager.h"

#include"cyy.h"
#include"leisai.h"
#include"doodle.h"

#include"0.quick_template.h"

#include"2.process_schedule.h"
#include"3.realtime_schedule.h"
#include"4.state_exchange.h"
#include"5.mutual_exclusion.h"
#include"6.synchronization.h"
#include"7.process_communication.h"
#include"8.deadlock.h"
#include"8.1deadlock_prevention.h"
#include"8.2deadlock_avoid.h"
#include"8.3deadlock_check_and_release.h"
#include"9.memory_allocate.h"
#include"9.in_and_out.h"
#include"10.es_visit.h"
#include"11.thread_mechanism.h"

#include <imgui.h>

#include<Windows.h>

ExampleManager::ExampleManager()
{

}

ExampleManager::~ExampleManager()
{
	if (current_example)
		current_example->on_exit();

	for (auto& pair : example_pool)
		delete pair.second;
}

void ExampleManager::add_example(Subject& subject, const MenuItem& menu_item, Example* example)
{
	subject.item_list.emplace_back(menu_item);
	example_pool[menu_item.id] = example;
}

void ExampleManager::on_init(SDL_Renderer* renderer)
{
	this->renderer = renderer;

	texture_target = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 720, 720);
	SDL_SetTextureBlendMode(texture_target, SDL_BLENDMODE_BLEND);
	texture_blank_content = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 600, 600);
	SDL_SetTextureBlendMode(texture_blank_content, SDL_BLENDMODE_BLEND);


	subject_creational.title = u8"一、WaWa歌舞厅";
	SDL_Texture* texture_icon_creational = ResourcesManager::instance()->find_texture("icon-creational");
	add_example(subject_creational, MenuItem("1example1", texture_icon_creational, u8"长夜月摇"), new CYY(renderer));
	add_example(subject_creational, MenuItem("1example2", texture_icon_creational, u8"蕾塞舞"), new LeiSai(renderer));
	add_example(subject_creational, MenuItem("1example3", texture_icon_creational, u8"Doodle Dance"), new Doodle(renderer));

	subject_structural.title = u8"二、功能拆解";
	SDL_Texture* texture_icon_structural = ResourcesManager::instance()->find_texture("icon-structural");
	add_example(subject_structural, MenuItem("2example1", texture_icon_structural, u8"1. 进程的控制"), new Example());
	add_example(subject_structural, MenuItem("2example2", texture_icon_structural, u8"2. 进程调度"), new ProcessSchedule());
	add_example(subject_structural, MenuItem("2example3", texture_icon_structural, u8"3. 实时调度"), new RealtimeSchedule());
	add_example(subject_structural, MenuItem("2example4", texture_icon_structural, u8"4. 状态切换"), new StateExchange());
	add_example(subject_structural, MenuItem("2example5", texture_icon_structural, u8"5. 互斥"), new MutexExclusion());
	add_example(subject_structural, MenuItem("2example6", texture_icon_structural, u8"6. 同步"), new Synchronization());
	add_example(subject_structural, MenuItem("2example7", texture_icon_structural, u8"7. 通信"), new ProcessCommunication());
	add_example(subject_structural, MenuItem("2example8", texture_icon_structural, u8"8. 死锁处理"), new DeadLock());
	add_example(subject_structural, MenuItem("2example8.1", texture_icon_structural, u8"8.1 预防死锁"), new DeadLockPrevention());
	add_example(subject_structural, MenuItem("2example8.2", texture_icon_structural, u8"8.2 避免死锁"), new DeadLockAvoid());
	add_example(subject_structural, MenuItem("2example8.3", texture_icon_structural, u8"8.3 检除死锁"), new DeadLockCheckAndRelease());
	add_example(subject_structural, MenuItem("2example9", texture_icon_structural, u8"9. 内存分配"), new MemoryAllocate());
	add_example(subject_structural, MenuItem("2example99", texture_icon_structural, u8"9. 换入换出"), new InAndOut());
	add_example(subject_structural, MenuItem("2example10", texture_icon_structural, u8"10. 外存访问"), new EsVisit());
	add_example(subject_structural, MenuItem("2example11", texture_icon_structural, u8"11. 线程机制"), new ThreadMechanism());
	add_example(subject_structural, MenuItem("2example12", texture_icon_structural, u8"0. 快速开发模板"), new QuickTemplate());
	
	//subject_behavioral.title = u8"三、完整演示";
	//SDL_Texture* texture_icon_behavioral = ResourcesManager::instance()->find_texture("icon-behavioral");	
	//add_example(subject_behavioral, MenuItem("3example1", texture_icon_behavioral, u8"1. 行为1"), new EEE1());
	//add_example(subject_behavioral, MenuItem("3example2", texture_icon_behavioral, u8"2. 行为2"), new Example());
}

void ExampleManager::on_update_blank_content()
{		
	ImGui::BeginChild("document", { ImGui::GetContentRegionAvail().x,640 }, ImGuiChildFlags_Border);

	//修改字体大小为30
	ImGui::SetWindowFontScale(1.5f);

	static float delta_h = 31;
	ImGui::Text(u8"执行状态:");
	ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(320, 40), 8, IM_COL32(0, 255, 0, 255));
	ImGui::Text(u8"活动阻塞:");
    ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(320, 40+delta_h*1), 8, IM_COL32(255, 0, 0, 255));
	ImGui::Text(u8"静止阻塞:");
	ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(320, 40 + delta_h * 2), 8, IM_COL32(95, 0, 0, 255));
	ImGui::Text(u8"活动就绪:");
    ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(320, 40 + delta_h * 3), 8, IM_COL32(255, 255, 0, 255));
	ImGui::Text(u8"静止就绪:");
	ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(320, 40 + delta_h * 4), 8, IM_COL32(95, 125, 0, 255));
	ImGui::Text(u8"终止状态:");
    ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(320, 40 + delta_h * 5), 8, IM_COL32(255, 0, 255, 255));
	ImGui::Text(u8"字体颜色: "); ImGui::SameLine();
	ImGui::ColorEdit3("color", (float*)&ImGui::GetStyle().Colors[ImGuiCol_Text]);

	ImGui::EndChild();

	ImGui::SetWindowFontScale(1.0f);
	

	ImGui::SetCursorPos({ 450,665 });//这里是写死了的
	ImGui::TextDisabled(u8"@WaWaJie");ImGui::SameLine();
	if (ImGui::ImageButton(ResourcesManager::instance()->find_texture("icon-bilibili"), { 14,14 }))
		ShellExecute(NULL, TEXT("open"), TEXT("https://space.bilibili.com/3546580678346970"), NULL, NULL, SW_SHOWNORMAL);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
        ImGui::Text(u8"打开我的bilibili主页");
        ImGui::EndTooltip();
	}ImGui::SameLine();
	if(ImGui::ImageButton(ResourcesManager::instance()->find_texture("CSDN"),{14,14}))
		ShellExecute(NULL, TEXT("open"), TEXT("https://blog.csdn.net/2301_79921853?type=blog"), NULL, NULL, SW_SHOWNORMAL);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(u8"打开我的CSDN主页");
		ImGui::EndTooltip();
	}ImGui::SameLine();
	if (ImGui::ImageButton(ResourcesManager::instance()->find_texture("icon-github"), { 14,14 }))
		ShellExecute(NULL, TEXT("open"), TEXT("https://github.com/WaWaJie"), NULL, NULL, SW_SHOWNORMAL);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(u8"打开我的GitHub主页");
		ImGui::EndTooltip();
	}ImGui::SameLine();
	if (ImGui::ImageButton(ResourcesManager::instance()->find_texture("now_coder_sis"), { 14,14 }))
		ShellExecute(NULL, TEXT("open"), TEXT("https://www.nowcoder.com/users/426830716"), NULL, NULL, SW_SHOWNORMAL);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(u8"打开我的牛客主页");
		ImGui::EndTooltip();
	}ImGui::SameLine();
	if (ImGui::ImageButton(ResourcesManager::instance()->find_texture("icon-cf"), { 14,14 }))
		ShellExecute(NULL, TEXT("open"), TEXT("https://mirror.codeforces.com/profile/WaWaJie?csrf_token=043507a27b7ab294db2097038aea9d9d"), NULL, NULL, SW_SHOWNORMAL);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(u8"打开我的CodeForces主页");
		ImGui::EndTooltip();
	}
}

void ExampleManager::switch_to(const std::string& id)
{
	if (id == "")
	{
		if (current_example)current_example->on_exit();
		current_example_id = "";
		current_example = nullptr;
	}

	if (current_example)
		current_example->on_exit();

	current_example_id = id;
	current_example = example_pool[id];

	if (current_example)
		current_example->on_enter();
}

void ExampleManager::on_input(const SDL_Event* event)
{
	//当前imgui没有接收信息
    if (ImGui::GetIO().WantCaptureKeyboard) return;

	//按下空格则当前样例置空
    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_SPACE)
    {
		switch_to("");
    }

	if (!current_example) return;

	current_example->on_input(event);
}

void ExampleManager::on_render()		
{
	if (!current_example)
	{
		//SDL_SetRenderTarget(renderer, texture_blank_content);
		//SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  //      SDL_RenderClear(renderer);
		////渲染一个圆形
  //      ImGui::GetWindowDrawList()->AddCircle(ImVec2(ImGui::GetWindowWidth() / 2, ImGui::GetWindowHeight() / 2), 100, IM_COL32(255, 0, 0, 255));
  //      ImGui::Render();
  //      SDL_RenderPresent(renderer);
		return;
	}
	SDL_SetRenderTarget(renderer, nullptr);

	current_example->on_render(renderer);
}

void ExampleManager::on_update(float delta)
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

	static const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoSavedSettings;

	ImGui::Begin("main_window", nullptr, flags);  // 窗口名称"main_window"，无关闭回调，应用上述标志
	{
		static const ImVec2 size_menu = { 180, ImGui::GetContentRegionAvail().y };
		ImGui::BeginChild("menu", size_menu, ImGuiChildFlags_Border);
		on_update_subject(subject_creational);    
		on_update_subject(subject_structural);    
		on_update_subject(subject_behavioral);    
		ImGui::EndChild();
	}
	ImGui::SameLine();
	
	// 定义内容区尺寸：占满剩余的所有空间（主窗口宽度 - 菜单宽度）
	static const ImVec2 size_stage = ImGui::GetContentRegionAvail();
	{
		// 创建带边框的子窗口"stage"，尺寸为剩余空间
		ImGui::BeginChild("stage", size_stage, ImGuiChildFlags_Border);
		if (current_example)
			current_example->on_update(delta);
		else
			on_update_blank_content();
		ImGui::EndChild();
	}
	ImGui::End();  // 结束主窗口绘制
}

void ExampleManager::on_update_subject(const Subject& subject)
{
	//创建可折叠标题栏
	if (ImGui::CollapsingHeader(subject.title.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (const MenuItem& menu_item : subject.item_list)
		{
			//定义图标尺寸，确保图标与文字尺寸一致
			static const ImVec2 size_icon =
			{
				ImGui::GetTextLineHeightWithSpacing(),
				ImGui::GetTextLineHeightWithSpacing()
			};

			ImGui::Image(menu_item.icon, size_icon); ImGui::SameLine();
			if (ImGui::Selectable(menu_item.title.c_str(), current_example_id == menu_item.id))
				switch_to(menu_item.id);
		}
	}
}
