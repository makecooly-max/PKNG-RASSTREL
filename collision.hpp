#pragma once
#include <SDL.h>
#include <vector>

class Collision {
public:
    static bool checkAABB(const SDL_Rect& a, const SDL_Rect& b);
    static bool resolveX(SDL_Rect& mover, float& velX, const std::vector<SDL_Rect>& walls);
    static bool resolveY(SDL_Rect& mover, float& velY, const std::vector<SDL_Rect>& walls);
};