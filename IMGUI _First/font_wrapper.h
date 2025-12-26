#ifndef _FONT_WRAPPER_H_
#define _FONT_WRAPPER_H_

#include <SDL_ttf.h>

#include <string>
#include <unordered_map>

class FontWrapper
{
public:
	FontWrapper(const std::string path)
	{
		this->path = path;
	}

	~FontWrapper()
	{
		for (auto& pair : font_pool)
			TTF_CloseFont(pair.second);
	}

	TTF_Font* get_font(int size)
	{
		TTF_Font* font = font_pool[size];

		if (!font)
		{
			//避免同意大小的字体多次加载
			font = TTF_OpenFont(path.c_str(), size);
			font_pool[size] = font;
		}

		return font;
	}

private:
	std::string path;
	std::unordered_map<int, TTF_Font*> font_pool;

};

#endif // !_FONT_WRAPPER_H_
