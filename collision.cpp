#include "collision.hpp"

bool Collision::checkAABB(const SDL_Rect& a, const SDL_Rect& b) {
    return SDL_HasIntersection(&a, &b) == SDL_TRUE;
}

bool Collision::resolveX(SDL_Rect& mover, float& velX, const std::vector<SDL_Rect>& walls) {
    bool hit = false;
    for (const auto& wall : walls) {
        if (checkAABB(mover, wall)) {
            if (velX > 0) mover.x = wall.x - mover.w;
            else if (velX < 0) mover.x = wall.x + wall.w;
            velX = 0;
            hit = true;
        }
    }
    return hit;
}

bool Collision::resolveY(SDL_Rect& mover, float& velY, const std::vector<SDL_Rect>& walls) {
    bool hit = false;
    for (const auto& wall : walls) {
        if (checkAABB(mover, wall)) {
            if (velY > 0) mover.y = wall.y - mover.h;
            else if (velY < 0) mover.y = wall.y + wall.h;
            velY = 0;
            hit = true;
        }
    }
    return hit;
}