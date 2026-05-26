#pragma once
#include <SDL.h>

class Camera {
public:
    Camera(int screenW, int screenH, int mapW, int mapH);

    void update(float targetX, float targetY, float dt);
    SDL_Rect worldToScreen(const SDL_Rect& worldRect) const;
    bool isVisible(const SDL_Rect& worldRect) const;

    float x = 0.f, y = 0.f;

private:
    int m_screenW, m_screenH;
    int m_mapW, m_mapH;
    static constexpr float LERP_SPEED = 8.0f;
};