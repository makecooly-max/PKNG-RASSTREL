#include "texturemanager.hpp"
#include <stdexcept>

TextureManager& TextureManager::getInstance() {
    static TextureManager instance;
    return instance;
}

void TextureManager::init(SDL_Renderer* renderer) {
    m_renderer = renderer;
}

SDL_Texture* TextureManager::load(const std::string& id, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) return nullptr;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);
    if (tex) m_textures[id] = tex;
    return tex;
}

SDL_Texture* TextureManager::get(const std::string& id) {
    auto it = m_textures.find(id);
    return (it != m_textures.end()) ? it->second : nullptr;
}

SDL_Texture* TextureManager::createSolid(const std::string& id, int w, int h, SDL_Color color) {
    SDL_Texture* tex = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!tex) return nullptr;
    SDL_SetRenderTarget(m_renderer, tex);
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_renderer);
    SDL_SetRenderTarget(m_renderer, nullptr);
    m_textures[id] = tex;
    return tex;
}

SDL_Texture* TextureManager::createCheckerboard(const std::string& id, int w, int h, SDL_Color c1, SDL_Color c2, int tileSize) {
    SDL_Texture* tex = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!tex) return nullptr;
    SDL_SetRenderTarget(m_renderer, tex);
    for (int y = 0; y < h; y += tileSize) {
        for (int x = 0; x < w; x += tileSize) {
            bool even = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            SDL_Color c = even ? c1 : c2;
            SDL_SetRenderDrawColor(m_renderer, c.r, c.g, c.b, c.a);
            SDL_Rect r = { x, y, tileSize, tileSize };
            SDL_RenderFillRect(m_renderer, &r);
        }
    }
    SDL_SetRenderTarget(m_renderer, nullptr);
    m_textures[id] = tex;
    return tex;
}

void TextureManager::clear() {
    for (auto& [id, tex] : m_textures) SDL_DestroyTexture(tex);
    m_textures.clear();
}