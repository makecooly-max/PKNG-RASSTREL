#include "camera.hpp"
#include <algorithm>
#include <cmath>

Camera::Camera(int screenW, int screenH, int mapW, int mapH)
    : m_screenW(screenW), m_screenH(screenH), m_mapW(mapW), m_mapH(mapH) {
}

void Camera::update(float targetX, float targetY, float dt) {
    float desiredX = targetX - m_screenW / 2.f;
    float desiredY = targetY - m_screenH / 2.f;

    float alpha = 1.f - std::exp(-LERP_SPEED * dt);
    x += (desiredX - x) * alpha;
    y += (desiredY - y) * alpha;

    x = std::max(0.f, std::min(x, (float)(m_mapW - m_screenW)));
    y = std::max(0.f, std::min(y, (float)(m_mapH - m_screenH)));
}

SDL_Rect Camera::worldToScreen(const SDL_Rect& worldRect) const {
    return {
        worldRect.x - (int)x,
        worldRect.y - (int)y,
        worldRect.w,
        worldRect.h
    };
}

bool Camera::isVisible(const SDL_Rect& worldRect) const {
    SDL_Rect screen = { (int)x, (int)y, m_screenW, m_screenH };
    SDL_Rect result;
    return SDL_IntersectRect(&worldRect, &screen, &result) == SDL_TRUE;
}