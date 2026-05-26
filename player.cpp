#include "player.hpp"
#include "camera.hpp"
#include "collision.hpp"
#include "texturemanager.hpp"
#include <algorithm>
#include <cmath>

Player::Player() {}

void Player::setCharacter(const CharacterDef& def) {
    m_name = def.name;
    m_textureId = def.textureId;
}

void Player::setPosition(int x, int y) { m_x = (float)x; m_y = (float)y; }

SDL_Rect Player::getBounds() const { return { (int)m_x, (int)m_y, WIDTH, HEIGHT }; }

void Player::handleInput(const Uint8* keys) {
    m_velX = 0.f; m_velY = 0.f;
    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])    m_velY = -SPEED;
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])  m_velY = SPEED;
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) { m_velX = -SPEED; }
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) { m_velX = SPEED; }
    if (m_velX != 0.f && m_velY != 0.f) {
        m_velX *= 0.7071f;
        m_velY *= 0.7071f;
    }
    if (aimDirX < 0) m_facing = 1; else m_facing = 0;
}

void Player::update(float dt, const std::vector<SDL_Rect>& walls, int mapW, int mapH) {
    if (m_velX != 0.f || m_velY != 0.f) {
        m_animTimer += dt;
        if (m_animTimer >= 0.12f) { m_animTimer = 0.f; m_animFrame = (m_animFrame + 1) % 4; }
    }
    else { m_animFrame = 0; m_animTimer = 0.f; }

    m_x += m_velX * dt;
    SDL_Rect b = getBounds();
    Collision::resolveX(b, m_velX, walls);
    m_x = (float)b.x;

    m_y += m_velY * dt;
    b = getBounds();
    Collision::resolveY(b, m_velY, walls);
    m_y = (float)b.y;

    m_x = std::max(0.f, std::min(m_x, (float)(mapW - WIDTH)));
    m_y = std::max(0.f, std::min(m_y, (float)(mapH - HEIGHT)));
}

void Player::render(SDL_Renderer* renderer, const Camera& camera) const {
    SDL_Rect worldRect = getBounds();
    SDL_Rect screenRect = camera.worldToScreen(worldRect);

    int cx = screenRect.x + screenRect.w / 2;
    int cy = screenRect.y + screenRect.h / 2;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 160);
    SDL_RenderDrawLine(renderer, cx, cy,
        (int)(cx + aimDirX * 40), (int)(cy + aimDirY * 40));

    SDL_Texture* tex = TextureManager::getInstance().get(m_textureId);
    if (tex) {
        int tw, th; SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
        int frameW = tw / 4;
        SDL_Rect src = { 0, 0, tw, th };
        SDL_RendererFlip flip = (m_facing == 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer, tex, &src, &screenRect, 0.0, nullptr, flip);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
        SDL_RenderFillRect(renderer, &screenRect);
        SDL_SetRenderDrawColor(renderer, 50, 120, 50, 255);
        SDL_Rect head = { screenRect.x + 6, screenRect.y, screenRect.w - 12, screenRect.h / 3 };
        SDL_RenderFillRect(renderer, &head);
    }
    SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);
    SDL_Rect muzz = { (int)(cx + aimDirX * 22) - 3, (int)(cy + aimDirY * 22) - 3, 6, 6 };
    SDL_RenderFillRect(renderer, &muzz);
}
