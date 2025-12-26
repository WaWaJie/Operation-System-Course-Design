#pragma once

#include<string>

#include<imgui.h>
#include<imgui_internal.h>
#include<imgui_impl_sdl.h>
#include<SDL.h>

enum class TextType
{
	Info,
	Warning,
	Error,
	Finish,
	KeyInfo
};

struct TextString
{
	std::string text_info = "";
	TextType type = TextType::Info;
	ImColor color = { 0,0,0,255 };
	
	TextString(const std::string& text, TextType type)
		:text_info(text), type(type)
	{
	}

};
