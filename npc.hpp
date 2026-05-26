#pragma once
#include <SDL.h>
#include <string>
#include <vector>

enum class NpcRole { Teacher, Student };

enum class NpcState { Idle, Alert, Flee, Dead };

struct BloodParticle {
    float x, y, vx, vy, life, maxLife;
    int size;
};

class Npc {
public:
    static constexpr int WIDTH = 32;
    static constexpr int HEIGHT = 48;

    Npc(const std::string& name, NpcRole role,
        const std::string& textureId, float x, float y, int hp, int floorIndex);

    void update(float dt, float playerX, float playerY,
        const std::vector<SDL_Rect>& walls, int mapW, int mapH);
    void render(SDL_Renderer* renderer, const class Camera& camera) const;
    void renderBlood(SDL_Renderer* renderer, const class Camera& camera) const;

    void takeDamage(int dmg, float bulletDirX, float bulletDirY);

    SDL_Rect getBounds() const;
    bool isDead()  const { return m_state == NpcState::Dead; }
    bool isAlive() const { return m_state != NpcState::Dead; }
    const std::string& getName() const { return m_name; }
    NpcRole getRole() const { return m_role; }
    int getFloor() const { return m_floor; }
    int getHp()    const { return m_hp; }
    int getMaxHp() const { return m_maxHp; }
    float getX()   const { return m_x; }
    float getY()   const { return m_y; }
    void  setPosition(float x, float y) { m_x = x; m_y = y; }

private:
    std::string m_name;
    NpcRole     m_role;
    std::string m_textureId;
    float m_x, m_y;
    float m_velX = 0, m_velY = 0;
    int   m_hp, m_maxHp;
    int   m_floor;
    NpcState m_state = NpcState::Idle;

    float m_idleTimer = 0.f;
    float m_moveTimer = 0.f;
    float m_idleDirX = 0.f;
    float m_idleDirY = 0.f;
    int   m_facing = 0;
    int   m_animFrame = 0;
    float m_animTimer = 0.f;
    float m_deadTimer = 0.f;

    mutable std::vector<BloodParticle> m_blood;
    void spawnBlood(float impactX, float impactY, float dirX, float dirY, int count);
    void updateBlood(float dt);
};