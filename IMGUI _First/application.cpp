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

