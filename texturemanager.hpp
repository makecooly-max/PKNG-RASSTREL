#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <unordered_map>

class TextureManager {
public:
    static TextureManager& getInstance();

    void init(SDL_Renderer* renderer);
    SDL_Texture* load(const std::string& id, const std::string& path);
    SDL_Texture* get(const std::string& id);
    SDL_Texture* createSolid(const std::string& id, int w, int h, SDL_Color color);
    SDL_Texture* createCheckerboard(const std::string& id, int w, int h, SDL_Color c1, SDL_Color c2, int tileSize);
    void clear();

private:
    TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    SDL_Renderer* m_renderer = nullptr;
    std::unordered_map<std::string, SDL_Texture*> m_textures;
};