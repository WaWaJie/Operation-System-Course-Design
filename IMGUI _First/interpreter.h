#pragma once

#include"example.h"

#include<lua.hpp>

#include<imgui.h>
#include<TextEditor.h>

class InterpreterPattern :public Example
{
public:
    InterpreterPattern(SDL_Renderer* renderer);
    ~InterpreterPattern();

	void on_update(float delta) override;

private:
    TextEditor code_editor;
    ImFont*font_code= nullptr;
	SDL_Texture* texture_target = nullptr;

};
