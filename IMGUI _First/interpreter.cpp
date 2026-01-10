#include"interpreter.h"

#include<lua.hpp>

#include<imgui_stdlib.h>
#include<SDL2_gfxPrimitives.h>
#include<iostream>


static SDL_Renderer* g_renderer;
static std::string str_buffet_log;

static int api_set_draw_color(lua_State* lua_state)
{
	SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(g_renderer,(Uint8)luaL_checkinteger(lua_state, 1), (Uint8)luaL_checkinteger(lua_state, 2), (Uint8)luaL_checkinteger(lua_state
		, 3), (Uint8)luaL_checkinteger(lua_state, 4));
	return 0;
}

static int api_draw_circle(lua_State*lua_state)
{
	SDL_Color color_painter;
	SDL_GetRenderDrawColor(g_renderer, &color_painter.r, &color_painter.g, &color_painter.b, &color_painter.a);
	int x = (int)lua_tointeger(lua_state, 1);
	int y = (int)lua_tointeger(lua_state, 2);
	int radius = (int)lua_tointeger(lua_state, 3);
	filledCircleRGBA(g_renderer, x, y, radius, color_painter.r, color_painter.g, color_painter.b, color_painter.a);
	return 0;
}

static int api_draw_rectangle(lua_State* lua_state)
{
	SDL_Rect rect =
	{
		(int)lua_tointeger(lua_state, 1),
		(int)lua_tointeger(lua_state, 2),
		(int)lua_tointeger(lua_state, 3),
		(int)lua_tointeger(lua_state, 4)
	};
	SDL_RenderFillRect(g_renderer, &rect);
	return 0;
}

static int api_draw_triangle(lua_State* lua_state)
{
	SDL_Color color_painter;
	SDL_GetRenderDrawColor(g_renderer, &color_painter.r, &color_painter.g, &color_painter.b, &color_painter.a);
	int x1 = (int)lua_tointeger(lua_state, 1);
	int y1 = (int)lua_tointeger(lua_state, 2);
	int x2 = (int)lua_tointeger(lua_state, 3);
	int y2 = (int)lua_tointeger(lua_state, 4);
	int x3 = (int)lua_tointeger(lua_state, 5);
	int y3 = (int)lua_tointeger(lua_state, 6);
	filledTrigonRGBA(g_renderer, x1, y1, x2, y2, x3, y3, color_painter.r, color_painter.g, color_painter.b, color_painter.a);
	return 0;
}

static int api_print(lua_State* lua_state)
{
	str_buffet_log += luaL_checkstring(lua_state, 1);
	str_buffet_log += "\n";
	return 0;
}



InterpreterPattern::InterpreterPattern(SDL_Renderer* renderer)
{
	g_renderer = renderer;
	texture_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 350, 350);

	ImGuiIO& ioImGui = ImGui::GetIO();
	font_code = ImGui::GetIO().Fonts->AddFontFromFileTTF("resources\SarasaMonoSC-Regular.ttf", 18.0f, nullptr, ioImGui.Fonts->GetGlyphRangesChineseSimplifiedCommon());

	TextEditor::LanguageDefinition lang_def = TextEditor::LanguageDefinition::Lua();
	{
		TextEditor::Identifier id_print;
        id_print.mDeclaration = u8"输出相应的内容";
		lang_def.mIdentifiers.insert(std::make_pair(u8"print", id_print));

		TextEditor::Identifier id_set_draw_color;
		id_set_draw_color.mDeclaration = u8"id_set_draw_color(r, g, b, a)：设置颜色";
		lang_def.mIdentifiers.insert(std::make_pair(u8"set_draw_color", id_set_draw_color));

		TextEditor::Identifier id_draw_rectangle;
		id_draw_rectangle.mDeclaration = u8"draw_rectangle(x,y,w,h)：绘制填充矩形";
		lang_def.mIdentifiers.insert(std::make_pair(u8"draw_rectangle", id_draw_rectangle));
	
		TextEditor::Identifier id_draw_circle;
		id_draw_circle.mDeclaration = u8"draw_circle(x,y,r)：绘制填充圆形";
		lang_def.mIdentifiers.insert(std::make_pair(u8"draw_circle", id_draw_circle));

		TextEditor::Identifier id_draw_triangle;
		id_draw_triangle.mDeclaration = u8"draw_triangle(x1,y1,x2,y2,x3,y3)：绘制填充三角形";
		lang_def.mIdentifiers.insert(std::make_pair(u8"draw_triangle", id_draw_triangle));
	}

	code_editor.SetLanguageDefinition(lang_def);
	code_editor.SetShowWhitespaces(false);
	code_editor.SetText("-- Lua Interpreter Example\nprint('Hello, Lua!')\n\nset_draw_color(255, 125, 75, 155)\ndraw_circle(150, 150, 100)\nset_draw_color(12, 250, 37, 65)\ndraw_rectangle(20, 20, 120, 120)\nset_draw_color(45,91,156,168)\ndraw_triangle(100, 250, 300, 50, 200, 350)\n");
	
	str_buffet_log = u8"等待执行Lua脚本...\n";

}
InterpreterPattern::~InterpreterPattern()
{
    SDL_DestroyTexture(texture_target);
}

void InterpreterPattern::on_update(float deleta)
{
	static const ImVec2 button_size = ImVec2(ImGui::GetContentRegionAvail().x, 30);
	if (ImGui::Button(u8"开始执行", button_size))
	{
		str_buffet_log = u8"=== Executing Lua Script === 【执行Lua脚本】\n";

		lua_State*lua_state= luaL_newstate();
        luaL_openlibs(lua_state);

		lua_pushcfunction(lua_state, api_set_draw_color);
        lua_setglobal(lua_state, "set_draw_color");
		lua_pushcfunction(lua_state, api_draw_circle);
        lua_setglobal(lua_state, "draw_circle");
        lua_pushcfunction(lua_state, api_draw_rectangle);
        lua_setglobal(lua_state, "draw_rectangle");
		lua_pushcfunction(lua_state, api_print);
        lua_setglobal(lua_state, "print");
		lua_pushcfunction(lua_state, api_draw_triangle);
		lua_setglobal(lua_state, "draw_triangle");
		
		SDL_SetRenderTarget(g_renderer, texture_target);
		SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
		SDL_RenderClear(g_renderer);
		
		if(luaL_dostring(lua_state, code_editor.GetText().c_str())!= LUA_OK)
		{
			str_buffet_log += u8"Lua Error ===== 【执行失败】: \n";
			str_buffet_log += lua_tostring(lua_state, -1);
			str_buffet_log += "\n";
		}

		SDL_SetRenderTarget(g_renderer, nullptr);


		lua_close(lua_state);
	}

	{
		ImGui::PushFont(font_code);
		static const ImVec2 code_editor_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30);
		code_editor.Render("code_editor", code_editor_size);
		ImGui::PopFont();

	}
	{
		ImGui::BeginGroup();
		ImGui::TextUnformatted(u8"绘图内容");		
		static const ImVec2 texture_size = ImVec2(350, 350);
		ImGui::BeginChild("render_result", texture_size, ImGuiChildFlags_Border);
		ImGui::Image(texture_target, ImGui::GetContentRegionAvail());
		ImGui::EndChild();
		ImGui::EndGroup();
	}
	ImGui::SameLine();
	{
		ImGui::BeginGroup();
		ImGui::TextUnformatted(u8"日志输出");
		static const ImVec2 log_size = ImVec2(350, 350);
		ImGui::InputTextMultiline("log_output", &str_buffet_log,log_size, ImGuiInputTextFlags_ReadOnly);
		ImGui::EndGroup();
	}
}