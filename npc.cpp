#include "npc.hpp"
#include "camera.hpp"
#include "collision.hpp"
#include "texturemanager.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

Npc::Npc(const std::string& name, NpcRole role,
    const std::string& textureId, float x, float y, int hp, int floorIndex)
    : m_name(name), m_role(role), m_textureId(textureId),
    m_x(x), m_y(y), m_hp(hp), m_maxHp(hp), m_floor(floorIndex)
{
}

SDL_Rect Npc::getBounds() const {
    return { (int)m_x, (int)m_y, WIDTH, HEIGHT };
}

static float dist(float ax, float ay, float bx, float by) {
    float dx = bx - ax, dy = by - ay;
    return std::sqrt(dx * dx + dy * dy);
}

void Npc::spawnBlood(float ix, float iy, float dirX, float dirY, int count) {
    for (int i = 0; i < count; ++i) {
        BloodParticle p;
        p.x = ix; p.y = iy;
        float spread = ((rand() % 200) - 100) / 100.f;
        float speed = 60.f + (rand() % 140);
        p.vx = dirX * speed + spread * 80.f;
        p.vy = dirY * speed + spread * 80.f + (rand() % 40);
        p.maxLife = p.life = 0.4f + (rand() % 40) / 100.f;
        p.size = 3 + rand() % 5;
        m_blood.push_back(p);
    }
}

void Npc::updateBlood(float dt) {
    for (auto& p : m_blood) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.vy += 200.f * dt;
        p.life -= dt;
    }
    m_blood.erase(std::remove_if(m_blood.begin(), m_blood.end(),
        [](const BloodParticle& p) { return p.life <= 0.f; }), m_blood.end());
}

void Npc::takeDamage(int dmg, float bDirX, float bDirY) {
    if (m_state == NpcState::Dead) return;
    m_hp -= dmg;
    float cx = m_x + WIDTH / 2.f;
    float cy = m_y + HEIGHT / 2.f;
    spawnBlood(cx, cy, bDirX, bDirY, 18 + rand() % 12);
    if (m_hp <= 0) {
        m_hp = 0;
        m_state = NpcState::Dead;
        m_deadTimer = 0.f;
    }
    else {
        m_state = NpcState::Alert;
    }
}

void Npc::update(float dt, float playerX, float playerY,
    const std::vector<SDL_Rect>& walls, int mapW, int mapH) {
    updateBlood(dt);
    if (m_state == NpcState::Dead) {
        m_deadTimer += dt;
        return;
    }

    float px = playerX, py = playerY;
    float d = dist(m_x + WIDTH / 2.f, m_y + HEIGHT / 2.f, px, py);

    if (m_state == NpcState::Idle) {
        if (d < 400.f) m_state = NpcState::Alert;
    }
    else if (m_state == NpcState::Alert) {
        if (d > 600.f) m_state = NpcState::Idle;
        if (d < 200.f) m_state = NpcState::Flee;
    }
    else if (m_state == NpcState::Flee) {
        if (d > 400.f) m_state = NpcState::Alert;
    }

    float speed = 0.f;
    m_velX = 0; m_velY = 0;

    if (m_state == NpcState::Idle) {
        speed = 60.f;
        m_idleTimer -= dt;
        m_moveTimer -= dt;
        if (m_idleTimer <= 0.f) {
            m_idleTimer = 1.5f + (rand() % 200) / 100.f;
            m_moveTimer = 0.8f + (rand() % 100) / 100.f;
            float angle = (rand() % 628) / 100.f;
            m_idleDirX = std::cos(angle);
            m_idleDirY = std::sin(angle);
        }
        if (m_moveTimer > 0.f) {
            m_velX = m_idleDirX * speed;
            m_velY = m_idleDirY * speed;
        }
    }
    else if (m_state == NpcState::Alert) {
        speed = 80.f;
        float dx = (px - m_x), dy = (py - m_y);
        float len = std::sqrt(dx * dx + dy * dy);
        if (len > 0.1f) { dx /= len; dy /= len; }
        m_velX = dx * speed;
        m_velY = dy * speed;
    }
    else if (m_state == NpcState::Flee) {
        speed = 160.f;
        float dx = (m_x - px), dy = (m_y - py);
        float len = std::sqrt(dx * dx + dy * dy);
        if (len > 0.1f) { dx /= len; dy /= len; }
        m_velX = dx * speed;
        m_velY = dy * speed;
    }

    if (m_velX < 0) m_facing = 1; else if (m_velX > 0) m_facing = 0;

    if (m_velX != 0.f || m_velY != 0.f) {
        m_animTimer += dt;
        if (m_animTimer >= 0.15f) { m_animTimer = 0.f; m_animFrame = (m_animFrame + 1) % 4; }
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

void Npc::renderBlood(SDL_Renderer* renderer, const Camera& camera) const {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (const auto& p : m_blood) {
        float alpha = std::max(0.f, p.life / p.maxLife);
        SDL_SetRenderDrawColor(renderer, 200, 0, 0, (Uint8)(alpha * 220));
        SDL_Rect r = { (int)(p.x - camera.x) - p.size / 2,
                       (int)(p.y - camera.y) - p.size / 2,
                       p.size, p.size };
        SDL_RenderFillRect(renderer, &r);
    }
}

void Npc::render(SDL_Renderer* renderer, const Camera& camera) const {
    SDL_Rect worldRect = getBounds();
    SDL_Rect screenRect = camera.worldToScreen(worldRect);

    if (!camera.isVisible(worldRect)) { renderBlood(renderer, camera); return; }

    if (m_state == NpcState::Dead) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_Texture* tex = TextureManager::getInstance().get(m_textureId);
        if (tex) {
            SDL_SetTextureAlphaMod(tex, 180);
            SDL_SetTextureColorMod(tex, 120, 40, 40);

            int tw, th;
            SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);


            SDL_Rect src = { 0, 0, tw, th };

            SDL_Rect dst = {
                screenRect.x,
                screenRect.y + screenRect.h / 3,
                screenRect.h * 0.7f,
                screenRect.w * 0.8f              
            };

            SDL_RenderCopyEx(renderer, tex, &src, &dst, 90.0, nullptr, SDL_FLIP_NONE);

            SDL_SetTextureAlphaMod(tex, 255);
            SDL_SetTextureColorMod(tex, 255, 255, 255);
        }
        else {
            SDL_SetRenderDrawColor(renderer, 100, 20, 20, 180);
            SDL_Rect flat = { screenRect.x, screenRect.y + screenRect.h / 3,
                              screenRect.w, screenRect.h / 2 };
            SDL_RenderFillRect(renderer, &flat);
        }
        renderBlood(renderer, camera);
        return;
    }

    SDL_Texture* tex = TextureManager::getInstance().get(m_textureId);
    if (tex) {
        int tw, th; SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
        int frameW = tw / 4;
        SDL_Rect src = { 0, 0, tw, th };
        SDL_RendererFlip flip = (m_facing == 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        if (m_state == NpcState::Alert)
            SDL_SetTextureColorMod(tex, 255, 180, 180);
        else if (m_state == NpcState::Flee)
            SDL_SetTextureColorMod(tex, 255, 220, 100);
        else
            SDL_SetTextureColorMod(tex, 255, 255, 255);

        SDL_RenderCopyEx(renderer, tex, &src, &screenRect, 0.0, nullptr, flip);
        SDL_SetTextureColorMod(tex, 255, 255, 255);
    }
    else {
        SDL_Color col;
        if (m_role == NpcRole::Teacher) col = { 80, 160, 230, 255 };
        else                             col = { 230, 180, 80, 255 };
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
        SDL_RenderFillRect(renderer, &screenRect);
        SDL_Rect head = { screenRect.x + 6, screenRect.y, screenRect.w - 12, screenRect.h / 3 };
        SDL_SetRenderDrawColor(renderer, col.r / 2, col.g / 2, col.b / 2, 255);
        SDL_RenderFillRect(renderer, &head);
    }

    if (m_hp < m_maxHp) {
        int bw = screenRect.w;
        int bh = 5;
        int bx = screenRect.x;
        int by = screenRect.y - 8;
        SDL_SetRenderDrawColor(renderer, 60, 0, 0, 220);
        SDL_Rect bgBar = { bx, by, bw, bh };
        SDL_RenderFillRect(renderer, &bgBar);
        int filled = (int)(bw * (float)m_hp / m_maxHp);
        SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
        SDL_Rect hpBar = { bx, by, filled, bh };
        SDL_RenderFillRect(renderer, &hpBar);
    }

    renderBlood(renderer, camera);
}
