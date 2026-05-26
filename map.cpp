#include "map.hpp"
#include "texturemanager.hpp"
#include <SDL_ttf.h>

bool Room::isDoor(const SDL_Rect& playerRect) const {
    return SDL_HasIntersection(&playerRect, &door) == SDL_TRUE;
}

Map::Map() {
    for (int i = 0; i < FLOOR_COUNT; ++i) buildFloor(i);
}

static int floorY(int floorIndex) {
    return Map::FLOOR_OFFSET_Y + floorIndex * (Map::FLOOR_HEIGHT + 60);
}

void Map::buildFloor(int index) {
    FloorData fd;
    fd.floorIndex = index;

    const int fy = floorY(index);
    const int fw = 2400;
    const int fh = FLOOR_HEIGHT;
    const int fx = 400;

    fd.floorRect = { fx, fy, fw, fh };
    const int wallT = 16;
    const int roomW = 380;
    const int roomH = 300;
    const int roomSpacing = 22;
    const int doorW = 56;
    const int doorH = 14;
    const int corridorW = 300;

    const int leftRoomX = fx + wallT;
    const int corridorX = leftRoomX + roomW;
    const int rightRoomX = corridorX + corridorW; 
    const int actualFW = wallT + roomW + corridorW + roomW + wallT;

    fd.corridorRect = { corridorX, fy, corridorW, fh };

    auto addWall = [&](SDL_Rect r) { fd.walls.push_back(r); };

    addWall({ fx, fy,                  actualFW, wallT });
    addWall({ fx, fy + fh - wallT,     actualFW, wallT });
    addWall({ fx, fy,                  wallT, fh });
    addWall({ fx + actualFW - wallT, fy, wallT, fh });
    addWall({ corridorX + corridorW - wallT, fy, wallT, fh });

    struct RoomDef { std::string label; bool isCanteen; bool isToilet; };
    std::vector<RoomDef> leftDefs, rightDefs;

    if (index == 0) {
        leftDefs = {
            { "Столова",    true,  false },
            { "Туалет (Ч)", false, true  },
            { "Туалет (Ж)", false, true  }
        };
    }
    else {
        for (int k = 0; k < 3; ++k)
            leftDefs.push_back({ "Каб. " + std::to_string((index + 1) * 100 + k + 1), false, false });
    }
    for (int k = 0; k < 3; ++k)
        rightDefs.push_back({ "Каб. " + std::to_string((index + 1) * 100 + 50 + k + 1), false, false });

    int startY = fy + wallT + roomSpacing;

    auto buildSide = [&](int rx, std::vector<RoomDef>& defs, bool leftSide) {
        for (int k = 0; k < (int)defs.size(); ++k) {
            int ry = startY + k * (roomH + roomSpacing);
            if (ry + roomH > fy + fh - wallT) break;

            SDL_Rect topW = { rx, ry,              roomW, wallT };
            SDL_Rect botW = { rx, ry + roomH - wallT, roomW, wallT };
            SDL_Rect leftW = { rx, ry,              wallT, roomH };
            SDL_Rect rightW = { rx + roomW - wallT,  ry,    wallT, roomH };

            int doorMid = ry + roomH / 2;
            int doorY = doorMid - doorW / 2;

            Room room;
            room.bounds = { rx, ry, roomW, roomH };
            room.label = defs[k].label;
            room.isCanteen = defs[k].isCanteen;
            room.isToilet = defs[k].isToilet;

            room.innerWalls.push_back(topW);
            room.innerWalls.push_back(botW);
            room.innerWalls.push_back(leftW);
            room.innerWalls.push_back(rightW);

            if (leftSide) {
                room.door = { rx + roomW - wallT, doorY, doorH, doorW };
                addWall(topW); addWall(botW); addWall(leftW);
                addWall({ rx + roomW - wallT, ry,            wallT, doorY - ry });
                addWall({ rx + roomW - wallT, doorY + doorW, wallT, ry + roomH - (doorY + doorW) });
                room.innerWalls.pop_back(); 
                room.innerWalls.push_back({ rx + roomW - wallT, ry,            wallT, doorY - ry });
                room.innerWalls.push_back({ rx + roomW - wallT, doorY + doorW, wallT, ry + roomH - (doorY + doorW) });
            }
            else {
                room.door = { rx, doorY, doorH, doorW };
                addWall(topW); addWall(botW); addWall(rightW);
                addWall({ rx, ry,            wallT, doorY - ry });
                addWall({ rx, doorY + doorW, wallT, ry + roomH - (doorY + doorW) });
                room.innerWalls.clear();
                room.innerWalls.push_back(topW);
                room.innerWalls.push_back(botW);
                room.innerWalls.push_back(rightW);
                room.innerWalls.push_back({ rx, ry,            wallT, doorY - ry });
                room.innerWalls.push_back({ rx, doorY + doorW, wallT, ry + roomH - (doorY + doorW) });
            }

            fd.rooms.push_back(std::move(room));
        }
        };

    buildSide(leftRoomX, leftDefs, true);
    buildSide(rightRoomX, rightDefs, false);

    const int stairW = 120, stairH = 80;
    const int stairX = corridorX + corridorW / 2 - stairW / 2;

    if (index > 0) {
        Staircase dn;
        dn.bounds = { stairX, fy + fh / 2 - stairH - 10, stairW, stairH };
        dn.fromFloor = index; dn.toFloor = index - 1;
        dn.spawnX = stairX + stairW / 2;
        dn.spawnY = floorY(index - 1) + Map::FLOOR_HEIGHT / 2;
        fd.staircases.push_back(dn);
    }
    if (index < FLOOR_COUNT - 1) {
        Staircase up;
        up.bounds = { stairX, fy + fh / 2 + 10, stairW, stairH };
        up.fromFloor = index; up.toFloor = index + 1;
        up.spawnX = stairX + stairW / 2;
        up.spawnY = floorY(index + 1) + Map::FLOOR_HEIGHT / 2;
        fd.staircases.push_back(up);
    }

    m_floors.push_back(std::move(fd));
}

const FloorData& Map::getFloor(int idx)         const { return m_floors[idx]; }
const std::vector<SDL_Rect>& Map::getWalls(int fi)          const { return m_floors[fi].walls; }
const std::vector<SDL_Rect>& Map::getRoomWalls(int fi, int ri) const { return m_floors[fi].rooms[ri].innerWalls; }
const std::vector<Room>& Map::getRooms(int fi)          const { return m_floors[fi].rooms; }
const std::vector<Staircase>& Map::getStaircases(int fi)     const { return m_floors[fi].staircases; }

void Map::drawLabel(SDL_Renderer* renderer, const Camera& camera, TTF_Font* font,
    const std::string& text, int wx, int wy) {
    if (!font) return;
    SDL_Color c = { 230,230,230,255 };
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), c);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect dst = { wx - (int)camera.x - surf->w / 2, wy - (int)camera.y - surf->h / 2, surf->w, surf->h };
    SDL_FreeSurface(surf);
    if (tex) { SDL_RenderCopy(renderer, tex, nullptr, &dst); SDL_DestroyTexture(tex); }
}

static void drawDesk(SDL_Renderer* renderer, const Camera& camera, int wx, int wy) {
    SDL_Rect desk = { wx - (int)camera.x - 22, wy - (int)camera.y - 10, 44, 22 };
    SDL_SetRenderDrawColor(renderer, 160, 110, 60, 255);
    SDL_RenderFillRect(renderer, &desk);
    SDL_SetRenderDrawColor(renderer, 100, 65, 30, 255);
    SDL_RenderDrawRect(renderer, &desk);
    SDL_SetRenderDrawColor(renderer, 80, 50, 20, 255);
    for (int dx : {-18, 18}) for (int dy : {-6, 10}) {
        SDL_Rect leg = { wx - (int)camera.x + dx - 2, wy - (int)camera.y + dy - 2, 4, 4 };
        SDL_RenderFillRect(renderer, &leg);
    }
    SDL_Rect chair = { wx - (int)camera.x - 10, wy - (int)camera.y + 16, 20, 18 };
    SDL_SetRenderDrawColor(renderer, 120, 80, 40, 255);
    SDL_RenderFillRect(renderer, &chair);
    SDL_SetRenderDrawColor(renderer, 80, 50, 20, 255);
    SDL_RenderDrawRect(renderer, &chair);
}

static void drawBlackboard(SDL_Renderer* renderer, const Camera& camera, int wx, int wy, int bw) {
    SDL_Rect frame = { wx - (int)camera.x - bw / 2 - 4, wy - (int)camera.y - 4, bw + 8, 28 };
    SDL_SetRenderDrawColor(renderer, 80, 55, 30, 255);
    SDL_RenderFillRect(renderer, &frame);
    SDL_Rect board = { wx - (int)camera.x - bw / 2, wy - (int)camera.y, bw, 20 };
    SDL_SetRenderDrawColor(renderer, 30, 90, 50, 255);
    SDL_RenderFillRect(renderer, &board);
    SDL_SetRenderDrawColor(renderer, 220, 220, 210, 200);
    SDL_RenderDrawLine(renderer,
        wx - (int)camera.x - bw / 2 + 8, wy - (int)camera.y + 7,
        wx - (int)camera.x - bw / 2 + bw / 3, wy - (int)camera.y + 7);
    SDL_RenderDrawLine(renderer,
        wx - (int)camera.x - bw / 2 + bw / 3 + 6, wy - (int)camera.y + 12,
        wx - (int)camera.x - bw / 2 + bw * 2 / 3, wy - (int)camera.y + 12);
}

void Map::renderRoom(SDL_Renderer* renderer, const Camera& camera,
    const FloorData& fd, int roomIdx, TTF_Font* font) {
    auto& tm = TextureManager::getInstance();
    SDL_Texture* wallTex = tm.get("wall_tile");
    SDL_Texture* doorTex = tm.get("door_tile");
    SDL_Texture* roomFloorTex = tm.get("room_floor");

    const Room& r = fd.rooms[roomIdx];

    SDL_Rect inner = { r.bounds.x + 16, r.bounds.y + 16, r.bounds.w - 32, r.bounds.h - 32 };
    if (camera.isVisible(r.bounds)) {
        SDL_Rect dst = camera.worldToScreen(r.bounds);
        SDL_RenderCopy(renderer, roomFloorTex, nullptr, &dst);
    }

    for (const auto& w : r.innerWalls) {
        if (!camera.isVisible(w)) continue;
        SDL_Rect dst = camera.worldToScreen(w);
        SDL_RenderCopy(renderer, wallTex, nullptr, &dst);
    }

    if (camera.isVisible(r.door)) {
        SDL_Rect dst = camera.worldToScreen(r.door);
        SDL_RenderCopy(renderer, doorTex, nullptr, &dst);
    }

    drawLabel(renderer, camera, font, r.label,
        r.bounds.x + r.bounds.w / 2, r.bounds.y + 28);

    if (!r.isToilet) {
        int boardW = r.isCanteen ? 0 : 180;
        if (!r.isCanteen && boardW > 0) {
            drawBlackboard(renderer, camera,
                r.bounds.x + r.bounds.w / 2,
                r.bounds.y + 28,
                boardW);
        }

        if (!r.isCanteen) {
            int deskStartX = r.bounds.x + 70;
            int deskStartY = r.bounds.y + 100;
            int deskSpacX = 90;
            int deskSpacY = 80;
            for (int row = 0; row < 2; ++row) {
                for (int col = 0; col < 3; ++col) {
                    int dx = deskStartX + col * deskSpacX;
                    int dy = deskStartY + row * deskSpacY;
                    drawDesk(renderer, camera, dx, dy);
                }
            }
        }
        else {
            int tableSpacX = 110;
            int tableStartX = r.bounds.x + 80;
            int tableStartY = r.bounds.y + 100;
            for (int col = 0; col < 3; ++col) {
                int dx = tableStartX + col * tableSpacX;
                drawDesk(renderer, camera, dx, tableStartY);
                drawDesk(renderer, camera, dx, tableStartY + 110);
            }
        }
    }
    else {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        for (int k = 0; k < 3; ++k) {
            SDL_Rect cabin = {
                r.bounds.x + 30 + k * 90 - (int)camera.x,
                r.bounds.y + 60 - (int)camera.y,
                70, 90
            };
            SDL_SetRenderDrawColor(renderer, 160, 160, 175, 200);
            SDL_RenderFillRect(renderer, &cabin);
            SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
            SDL_RenderDrawRect(renderer, &cabin);
        }
    }
}

void Map::renderFloor(SDL_Renderer* renderer, const Camera& camera,
    const FloorData& fd, TTF_Font* font, int insideRoomIdx) {
    auto& tm = TextureManager::getInstance();
    SDL_Texture* floorTex = tm.get("floor_tile");
    SDL_Texture* wallTex = tm.get("wall_tile");
    SDL_Texture* doorTex = tm.get("door_tile");
    SDL_Texture* stairTex = tm.get("stair_tile");
    SDL_Texture* roomFloorTex = tm.get("room_floor");
    SDL_Texture* corridorTex = tm.get("corridor_tile");

    auto drawTiled = [&](SDL_Texture* tex, const SDL_Rect& wr) {
        if (!camera.isVisible(wr)) return;
        SDL_Rect dst = camera.worldToScreen(wr);
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        };

    if (insideRoomIdx >= 0 && insideRoomIdx < (int)fd.rooms.size()) {
        renderRoom(renderer, camera, fd, insideRoomIdx, font);
        return;
    }

    drawTiled(floorTex, fd.floorRect);
    drawTiled(corridorTex, fd.corridorRect);

    for (const auto& room : fd.rooms) {
        SDL_Rect inner = { room.bounds.x + 16, room.bounds.y + 16,
                           room.bounds.w - 32, room.bounds.h - 32 };
        drawTiled(roomFloorTex, inner);
    }
    for (const auto& wall : fd.walls) {
        if (!camera.isVisible(wall)) continue;
        SDL_Rect dst = camera.worldToScreen(wall);
        SDL_RenderCopy(renderer, wallTex, nullptr, &dst);
    }
    for (const auto& room : fd.rooms) {
        if (!camera.isVisible(room.door)) continue;
        SDL_Rect dst = camera.worldToScreen(room.door);
        SDL_RenderCopy(renderer, doorTex, nullptr, &dst);
        int cx = room.bounds.x + room.bounds.w / 2;
        int cy = room.bounds.y + room.bounds.h / 2;
        drawLabel(renderer, camera, font, room.label, cx, cy);
    }
    for (const auto& stair : fd.staircases) {
        if (!camera.isVisible(stair.bounds)) continue;
        SDL_Rect dst = camera.worldToScreen(stair.bounds);
        SDL_RenderCopy(renderer, stairTex, nullptr, &dst);
        std::string lbl = stair.toFloor > stair.fromFloor ? "Вверх ^" : "Вниз v";
        drawLabel(renderer, camera, font, lbl,
            stair.bounds.x + stair.bounds.w / 2, stair.bounds.y + stair.bounds.h / 2);
    }

    drawLabel(renderer, camera, font,
        "Поверх " + std::to_string(fd.floorIndex + 1),
        fd.floorRect.x + 80, fd.floorRect.y + 30);
}

void Map::render(SDL_Renderer* renderer, const Camera& camera, int currentFloor,
    TTF_Font* font, int insideRoomIdx) {
    renderFloor(renderer, camera, m_floors[currentFloor], font, insideRoomIdx);
}