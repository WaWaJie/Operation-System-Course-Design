#include"application.h"
#include"example_manager.h"

#include<sstream>
#include<iomanip>

Application::Application()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	Mix_Init(MIX_INIT_MP3);
	TTF_Init();

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

	window = SDL_CreateWindow(u8"操作系统",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		900, 720, SDL_WINDOW_ALLOW_HIGHDPI);

	renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.FrameBorderSize = 1.0f;
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
	style.FrameRounding = 2.0f;

	ImGuiIO& ioImGui = ImGui::GetIO();
	ioImGui.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	ioImGui.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\msyh.ttc)", 18.0f, nullptr, ioImGui.Fonts->GetGlyphRangesChineseFull());

	ResourcesManager::instance()->load(renderer);
	ExampleManager::instance()->on_init(renderer);

	timer_update_title.set_one_shot(false);
	timer_update_title.set_wait_time(0.1f);
	timer_update_title.set_on_timeout([&]()
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision(2) << fps_realtime;
			std::string title = u8"操作系统---FPS: " + ss.str();
			SDL_SetWindowTitle(window, title.c_str());
		});
}

Application::~Application()
{
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

int Application::run(int argc, char** argv)
{
	using namespace std::chrono;

	const nanoseconds frame_duration(1000000000 / 60);
	steady_clock::time_point last_tick = steady_clock::now();

	while (!is_quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);

			if (event.type == SDL_QUIT)
				is_quit = true;

			ExampleManager::instance()->on_input(&event);
		}

		steady_clock::time_point frame_start = steady_clock::now();
		float delta = duration<float>(frame_start - last_tick).count();

		fps_realtime = (float)1.0 / delta;
		timer_update_title.on_update(delta);
		
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ExampleManager::instance()->on_update(delta);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		ExampleManager::instance()->on_render();

		ImGui::Render();//生成绘制命令
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());//执行绘制命令
		
		SDL_RenderPresent(renderer);

		last_tick = frame_start;
		nanoseconds sleep_duration = frame_duration - (steady_clock::now() - frame_start);
		if (sleep_duration > nanoseconds(0))
			std::this_thread::sleep_for(sleep_duration);
	}

	return 0;
}

/*
1.      如何理解测试、质量管理、质量控制这几个相关的概念？
（1）测试：是质量控制的具体技术手段，通过设计用例、执行验证，模拟用户场景发现产品缺陷，验证功能、性能等是否满足要求，目的是发现已有问题，为质量改进提供依据。
（2）质量管理：是全生命周期、系统性的管理体系，核心是通过建立质量方针、流程和标准，从需求、设计、开发到交付的全环节保障产品质量，目标是预防质量问题，持续提升质量水平。
（3）质量控制：是质量管理的核心执行缓解，聚焦于过长和成果的“检查与纠偏”，通过监控生产/开发过程、检验产品输出，识别不符合标准的问题并及时整改，目标是剔除不合格品，确保最终交付符合要求。

2.      如何度量软件的质量？
（1）产品质量度量：含内部质量（代码复杂度、耦合度等）、外部质量（功能、性能、可靠性等）、使用质量（用户满意度、任务效率等）
（2）过程质量度量：核心指标为缺陷排除效率，常用GQM模型（目标-问题-度量）评估过程能力
（3）质量模型支撑：依赖McCall、Boehm、ISO9126等模型，明确质量指标与因素的对应关系。

3.      软件测试有哪些原则？
不能做穷举测试，越早越好，软件测试是有风险的，不修复原则，无法显示潜伏的缺陷，所有文档都需要进行测试，确定测试何时结束，缺陷的集群性，杀虫剂悖论（免疫力），小规模--》大规模进行测试，第三方原则。

4.      测试用例包含哪些信息，它在测试中起到什么作用？
（1）包含信息：
①基础标识：用例ID、测试模块/功能点；
②执行前提：前置条件（环境配置、数据准备）
③核心要素：测试步骤、测试数据
④判断标准：预期结果
⑤辅助信息：优先级、测试环境、适用版本
（2）核心作用
①指导测试实施，明确测试流程与数据准备
②标准化测试行为，避免操作随意性和需求遗漏
③作为评估测试结果的基准，判定执行结果是否合格
④支持复用与追溯，助力回归测试和团队协作
⑤为缺陷分析提供依据，明确问题定位方向


5.      有哪些工具可以计算代码覆盖度？代码覆盖度有什么局限性？
（1）工具：Junit，Gcov，Coverage等
（2）局限性：
①覆盖度高≠无缺陷：无法检验逻辑错误
②不反应需求覆盖
③难以覆盖复杂场景:并发异常等
④不涵盖非代码元素
⑤指标存在片面性

6.      课上介绍了很多黑盒测试方法，在实际应用中如何选择这些测试方法呢？请举例说明。
（1）等价类划分：输入取值范围广，比如水电费收费标准
（2）边界值分析法：含易出错的临界值，比如商品数量
（3）因果图+判定表：多条件组合决定结果，比如订单折扣（会员等级+金额+新用户）
（4）场景法：多步骤流程类功能，比如购物下单
（5）错误推测法：经验总结性错误，比如登入输入空账号
（6）正交试验法：多因子组合，比如视频播放（分辨率+速率+格式）

7.      软件在不断演化，在此过程中需要对测试用例进行有效的管理和维护。代码修改后要进行回归测试，此时需要对原有的测试用例集合做哪些修改呢（从对测试用例的增加、删除、修改三个方面进行阐述）。
（1）增加：补充覆盖新功能的用例；新增验证缺陷修复效果的用例；增加覆盖代码修改所影响关联模块的用例
（2）删除：移除因功能删除、需求变更而失效的用例；删掉重复冗余、无实际测试价值的用例
（3）修改：更新因代码逻辑调整而会导致预期结果或测试步骤过时的用例。


*/