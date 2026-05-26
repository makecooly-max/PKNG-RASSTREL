#pragma once
#include <SDL.h>
#include <string>
#include <vector>

enum class CharacterType {
    Janna,
    Vorona,
    Popovich,
    Char4,
    Char5,
    Char6
};

struct CharacterDef {
    CharacterType type;
    std::string   name;
    std::string   textureId;
    std::string   texturePath; 
};

class Player {
public:
    static constexpr int   WIDTH = 32;
    static constexpr int   HEIGHT = 48;
    static constexpr float SPEED = 200.f;

    Player();
    void setCharacter(const CharacterDef& def);
    void handleInput(const Uint8* keys);
    void update(float dt, const std::vector<SDL_Rect>& walls, int mapW, int mapH);
    void render(SDL_Renderer* renderer, const class Camera& camera) const;

    void setPosition(int x, int y);
    SDL_Rect getBounds() const;
    float getX() const { return m_x; }
    float getY() const { return m_y; }
    const std::string& getName() const { return m_name; }

    float aimDirX = 1.f, aimDirY = 0.f;
    float muzzleX() const { return m_x + WIDTH / 2.f + aimDirX * 20.f; }
    float muzzleY() const { return m_y + HEIGHT / 2.f + aimDirY * 20.f; }

private:
    float m_x = 1200.f, m_y = 200.f;
    float m_velX = 0.f, m_velY = 0.f;
    std::string m_name;
    std::string m_textureId;
    int   m_animFrame = 0;
    float m_animTimer = 0.f;
    int   m_facing = 0;
};
