#include"resources_manager.h"

#include<SDL_image.h>
#include<iostream>

void ResourcesManager::load(SDL_Renderer* renderer)
{
    using namespace std::filesystem;

    this->renderer = renderer;

    for (const auto& entry : directory_iterator("resources"))
    {
        if (entry.is_regular_file())
        {
            const path& path = entry.path();
            auto itor = loader_pool.find(path.extension().u8string());
            if (itor != loader_pool.end())
                itor->second(path);
        }
    }
}

ResourcesManager::ResourcesManager()
{
    loader_pool[".jpg"] = loader_pool[".png"] =
        [&](const std::filesystem::path& path)
        {
            SDL_Texture* texture = IMG_LoadTexture(renderer, path.u8string().c_str());
            texture_resid_list.push_back(path.stem().u8string());
            texture_pool[path.stem().u8string()] = texture;
        };

    loader_pool[".wav"] = loader_pool[".mp3"] =
        [&](const std::filesystem::path& path)
        {
            Mix_Chunk* audio = Mix_LoadWAV(path.u8string().c_str());
            audio_resid_list.push_back(path.stem().u8string());
            audio_pool[path.stem().u8string()] = audio;
        };

    loader_pool[".ttf"] = loader_pool[".otf"] =
        [&](const std::filesystem::path& path)
        {
            FontWrapper* font = new FontWrapper(path.u8string().c_str());
            font_resid_list.push_back(path.stem().u8string());
            font_pool[path.stem().u8string()] = font;
        };
    loader_pool[".gif"] =
        [&](const std::filesystem::path& path)
        {
            IMG_Animation* animation = IMG_LoadAnimation(path.u8string().c_str());
            animation_resid_list.push_back(path.stem().u8string());
            animation_pool[path.stem().u8string()] = animation;
        };
}

ResourcesManager::~ResourcesManager()
{
    for (auto& pair : font_pool)
        delete pair.second;
    for (auto& pair : audio_pool)
        Mix_FreeChunk(pair.second);
    for (auto& pair : texture_pool)
        SDL_DestroyTexture(pair.second);
}
