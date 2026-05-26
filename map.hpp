#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include "camera.hpp"

struct Room {
    SDL_Rect bounds;
    SDL_Rect door;
    std::string label;
    bool isCanteen = false;
    bool isToilet = false;
    bool isDoor(const SDL_Rect& playerRect) const;
    std::vector<SDL_Rect> innerWalls;
};

struct Staircase {
    SDL_Rect bounds;
    int fromFloor;
    int toFloor;
    int spawnX;
    int spawnY;
};

struct FloorData {
    int floorIndex;
    std::vector<SDL_Rect> walls;
    std::vector<Room>     rooms;
    std::vector<Staircase> staircases;
    SDL_Rect floorRect;
    SDL_Rect corridorRect;
};

class Map {
public:
    static constexpr int MAP_W = 3200;
    static constexpr int MAP_H = 3200;
    static constexpr int FLOOR_HEIGHT = 700;
    static constexpr int FLOOR_OFFSET_Y = 100;
    static constexpr int FLOOR_COUNT = 4;

    Map();
    void render(SDL_Renderer* renderer, const Camera& camera, int currentFloor,
        TTF_Font* font, int insideRoomIdx = -1);
    const FloorData& getFloor(int index)        const;
    const std::vector<SDL_Rect>& getWalls(int floorIndex)   const;
    const std::vector<SDL_Rect>& getRoomWalls(int fi, int ri) const;
    const std::vector<Room>& getRooms(int fi)           const;
    const std::vector<Staircase>& getStaircases(int fi)      const;

private:
    std::vector<FloorData> m_floors;

    void buildFloor(int index);
    void renderFloor(SDL_Renderer* renderer, const Camera& camera,
        const FloorData& fd, TTF_Font* font, int insideRoomIdx);
    void renderRoom(SDL_Renderer* renderer, const Camera& camera,
        const FloorData& fd, int roomIdx, TTF_Font* font);
    void drawLabel(SDL_Renderer* renderer, const Camera& camera, TTF_Font* font,
        const std::string& text, int wx, int wy);
};
