#include "game.hpp"
#include "map.hpp"
#include "player.hpp"
#include "camera.hpp"
#include "menu.hpp"
#include "texturemanager.hpp"
#include "collision.hpp"
#include "inventory.hpp"
#include "npc.hpp"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>

Game::Game() { srand((unsigned)time(nullptr)); }
Game::~Game() { shutdown(); }

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    if (IMG_Init(IMG_INIT_PNG) == 0)   return false;
    if (TTF_Init() != 0)               return false;

    m_window = SDL_CreateWindow("ПКНГ Shooter",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    if (!m_window) return false;

    m_renderer = SDL_CreateRenderer(m_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) return false;

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    m_font = TTF_OpenFont("assets/fonts/font.ttf", 22);
    m_smallFont = TTF_OpenFont("assets/fonts/font.ttf", 14);

    TextureManager::getInstance().init(m_renderer);
    loadTextures();

    m_map = std::make_unique<Map>();
    m_player = std::make_unique<Player>();
    m_camera = std::make_unique<Camera>(SCREEN_W, SCREEN_H, Map::MAP_W, Map::MAP_H);
    m_menu = std::make_unique<Menu>();
    m_menu->init(m_renderer, m_font, m_smallFont);
    m_inventory = std::make_unique<Inventory>();

    m_running = true;
    return true;
}


void Game::loadTextures() {
    auto& tm = TextureManager::getInstance();
    auto tryLoad = [&](const std::string& id, const std::string& path) {
        return tm.load(id, path) != nullptr;
        };


    tryLoad("char_janna", "assets/textures/janna.png");
    tryLoad("char_vorona", "assets/textures/vorona.png");
    tryLoad("char_popovich", "assets/textures/popovich.png");
    tryLoad("char_4", "assets/textures/char4.png");
    tryLoad("char_5", "assets/textures/char5.png");
    tryLoad("char_6", "assets/textures/char6.png");

    if (!tryLoad("floor_tile", "assets/textures/floor.png"))
        tm.createCheckerboard("floor_tile", 64, 64, { 210,195,170,255 }, { 195,180,155,255 }, 32);
    if (!tryLoad("wall_tile", "assets/textures/wall.png"))
        tm.createSolid("wall_tile", 64, 64, { 80,75,70,255 });
    if (!tryLoad("door_tile", "assets/textures/door.png"))
        tm.createSolid("door_tile", 40, 12, { 120,70,30,255 });
    if (!tryLoad("stair_tile", "assets/textures/stair.png"))
        tm.createCheckerboard("stair_tile", 120, 80, { 160,140,100,255 }, { 140,120,80,255 }, 20);
    if (!tryLoad("room_floor", "assets/textures/room_floor.png"))
        tm.createCheckerboard("room_floor", 64, 64, { 235,225,200,255 }, { 220,210,185,255 }, 32);
    if (!tryLoad("corridor_tile", "assets/textures/corridor.png"))
        tm.createCheckerboard("corridor_tile", 64, 64, { 190,185,175,255 }, { 175,170,160,255 }, 32);
}

void Game::spawnNpcs() {
    m_npcs.clear();

    const float corridorCX = 400.f + 16 + 380 + 150; 
    auto fy = [](int fi) {
        return (float)(Map::FLOOR_OFFSET_Y + fi * (Map::FLOOR_HEIGHT + 60) + Map::FLOOR_HEIGHT / 2);
        };

    const char* textures[] = {
        "char_janna", "char_vorona", "char_popovich",
        "char_4", "char_5", "char_6"
    };

    m_npcs.emplace_back("Жанна", NpcRole::Teacher, "char_janna", corridorCX - 60, fy(0) - 120, 100, 0);
    m_npcs.emplace_back("Ворона", NpcRole::Teacher, "char_vorona", corridorCX + 60, fy(0) + 80, 120, 0);
    m_npcs.emplace_back("Попович", NpcRole::Student, "char_popovich", corridorCX, fy(0), 80, 0);
    m_npcs.emplace_back("Супрун", NpcRole::Student, "char_4", corridorCX - 90, fy(0) + 60, 75, 0);
    m_npcs.emplace_back("Бездрапко", NpcRole::Student, "char_5", corridorCX + 90, fy(0) - 60, 75, 0);
    m_npcs.emplace_back("Плескач", NpcRole::Student, "char_6", corridorCX + 0, fy(0) + 130, 70, 0);

    m_npcs.emplace_back("Жанна 2", NpcRole::Teacher, "char_janna", corridorCX - 60, fy(1), 100, 1);
    m_npcs.emplace_back("Попович 2", NpcRole::Student, "char_popovich", corridorCX + 60, fy(1) + 50, 80, 1);
    m_npcs.emplace_back("Ворона 2", NpcRole::Teacher, "char_vorona", corridorCX - 30, fy(1) - 60, 120, 1);
    m_npcs.emplace_back("Супрун 2", NpcRole::Student, "char_4", corridorCX + 30, fy(1) - 90, 75, 1);
    m_npcs.emplace_back("Бездрапко 2", NpcRole::Student, "char_5", corridorCX - 80, fy(1) + 100, 75, 1);
    m_npcs.emplace_back("Плескач 2", NpcRole::Student, "char_6", corridorCX + 80, fy(1) + 120, 70, 1);

    m_npcs.emplace_back("Ворона 3", NpcRole::Teacher, "char_vorona", corridorCX, fy(2), 120, 2);
    m_npcs.emplace_back("Попович 3", NpcRole::Student, "char_popovich", corridorCX - 80, fy(2) - 40, 80, 2);
    m_npcs.emplace_back("Попович 4", NpcRole::Student, "char_popovich", corridorCX + 40, fy(2) + 50, 80, 2);
    m_npcs.emplace_back("Жанна 3", NpcRole::Teacher, "char_janna", corridorCX - 50, fy(2) + 110, 100, 2);
    m_npcs.emplace_back("Супрун 3", NpcRole::Student, "char_4", corridorCX + 100, fy(2) - 80, 75, 2);
    m_npcs.emplace_back("Плескач 3", NpcRole::Student, "char_6", corridorCX - 100, fy(2) + 80, 70, 2);

    m_npcs.emplace_back("Жанна 4", NpcRole::Teacher, "char_janna", corridorCX + 40, fy(3) + 40, 100, 3);
    m_npcs.emplace_back("Попович 5", NpcRole::Student, "char_popovich", corridorCX - 40, fy(3), 80, 3);
    m_npcs.emplace_back("Ворона 4", NpcRole::Teacher, "char_vorona", corridorCX + 80, fy(3) - 60, 120, 3);
    m_npcs.emplace_back("Бездрапко 3", NpcRole::Student, "char_5", corridorCX - 80, fy(3) + 100, 75, 3);
    m_npcs.emplace_back("Супрун 4", NpcRole::Student, "char_4", corridorCX + 110, fy(3) + 90, 75, 3);
    m_npcs.emplace_back("Плескач 4", NpcRole::Student, "char_6", corridorCX - 110, fy(3) - 80, 70, 3);

    const char* teacherTex[] = { "char_janna", "char_vorona" };
    const char* studentTex[] = { "char_popovich", "char_4", "char_5", "char_6" };
    int tIdx = 0, sIdx = 0;

    for (int fi = 0; fi < Map::FLOOR_COUNT; ++fi) {
        const auto& rooms = m_map->getFloor(fi).rooms;
        for (const auto& room : rooms) {
            if (room.isToilet) continue;
            float rx = (float)room.bounds.x;
            float ry = (float)room.bounds.y;
            float rw = (float)room.bounds.w;
            float rh = (float)room.bounds.h;

            if (room.isCanteen) {
                m_npcs.emplace_back(room.label + " Відв.1", NpcRole::Student,
                    studentTex[sIdx % 4], rx + rw * 0.3f, ry + rh * 0.4f, 70, fi);
                m_npcs.emplace_back(room.label + " Відв.2", NpcRole::Student,
                    studentTex[(sIdx + 1) % 4], rx + rw * 0.6f, ry + rh * 0.6f, 70, fi);
                sIdx += 2;
            }
            else {
                m_npcs.emplace_back(room.label + " Вч.", NpcRole::Teacher,
                    teacherTex[tIdx % 2], rx + rw * 0.5f - 16, ry + 55, 100, fi);
                m_npcs.emplace_back(room.label + " Учень1", NpcRole::Student,
                    studentTex[sIdx % 4], rx + rw * 0.28f, ry + rh * 0.5f, 80, fi);
                m_npcs.emplace_back(room.label + " Учень2", NpcRole::Student,
                    studentTex[(sIdx + 1) % 4], rx + rw * 0.65f, ry + rh * 0.65f, 80, fi);
                ++tIdx; sIdx += 2;
            }
        }
    }
}

void Game::startGame() {
    m_currentFloor = 0;
    m_insideRoomIdx = -1;
    m_kills = 0;
    m_bullets.clear();

    const auto& floorData = m_map->getFloor(0);
    int startX = floorData.corridorRect.x + floorData.corridorRect.w / 2 - Player::WIDTH / 2;
    int startY = floorData.floorRect.y + floorData.floorRect.h / 2 - Player::HEIGHT / 2;

    m_player->setCharacter(m_menu->getSelectedCharacter());
    m_player->setPosition(startX, startY);
    m_camera->x = std::max(0.f, (float)(startX - SCREEN_W / 2));
    m_camera->y = std::max(0.f, (float)(startY - SCREEN_H / 2));

    m_inventory = std::make_unique<Inventory>();
    spawnNpcs();
}

void Game::run() {
    using clock = std::chrono::high_resolution_clock;
    auto prev = clock::now();
    const float frameTime = 1.f / TARGET_FPS;

    while (m_running) {
        auto  now = clock::now();
        float dt = std::chrono::duration<float>(now - prev).count();
        prev = now;
        dt = std::min(dt, 0.05f);

        handleEvents();
        if (m_state == GameState::Playing) update(dt);
        render();

        float elapsed = std::chrono::duration<float>(clock::now() - now).count();
        if (elapsed < frameTime)
            SDL_Delay((Uint32)((frameTime - elapsed) * 1000.f));
    }
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { m_running = false; return; }

        if (m_state == GameState::Menu) {
            m_menu->handleEvent(e);
            if (m_menu->hasSelectedCharacter()) {
                startGame();
                m_state = GameState::Playing;
            }
        }
        else if (m_state == GameState::Playing) {
            if (e.type == SDL_KEYDOWN) {
                auto sym = e.key.keysym.sym;
                if (sym == SDLK_ESCAPE) { m_state = GameState::Paused; return; }
                if (sym == SDLK_r && !m_rWasPressed) {
                    if (auto* w = m_inventory->getCurrentWeapon()) w->reload();
                    m_rWasPressed = true;
                }
                if (sym == SDLK_q)   m_inventory->selectPrev();
                if (sym == SDLK_TAB) m_inventory->selectNext();
                if (sym == SDLK_1)   m_inventory->selectSlot(0);
                if (sym == SDLK_2)   m_inventory->selectSlot(1);
                if (sym == SDLK_3)   m_inventory->selectSlot(2);
            }
            if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_r)
                m_rWasPressed = false;
        }
        else if (m_state == GameState::Paused) {
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) m_state = GameState::Playing;
                if (e.key.keysym.sym == SDLK_F1) {
                    m_state = GameState::Menu;
                    m_menu->resetSelection();
                }
            }
        }
    }
}

void Game::checkInteractions() {
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    bool ePressed = keys[SDL_SCANCODE_E] || keys[SDL_SCANCODE_RETURN];
    m_interactHint.clear();

    SDL_Rect pb = m_player->getBounds();

    if (m_insideRoomIdx >= 0) {
        const auto& rooms = m_map->getRooms(m_currentFloor);
        if (m_insideRoomIdx < (int)rooms.size()) {
            const Room& room = rooms[m_insideRoomIdx];

            bool leftSideDoor = (room.door.x <= room.bounds.x + 20);
            int exitZoneSize = 50;
            SDL_Rect exitZone;

            if (leftSideDoor) {
                exitZone = { room.bounds.x, room.bounds.y + room.bounds.h / 2 - exitZoneSize,
                             exitZoneSize, exitZoneSize * 2 };
            }
            else {
                exitZone = { room.bounds.x + room.bounds.w - exitZoneSize,
                             room.bounds.y + room.bounds.h / 2 - exitZoneSize,
                             exitZoneSize, exitZoneSize * 2 };
            }

            if (Collision::checkAABB(pb, exitZone)) {
                m_interactHint = "[E] Вийти з " + room.label;
                if (ePressed && !m_eWasPressed) {
                    m_insideRoomIdx = -1;

                    const FloorData& floorData = m_map->getFloor(m_currentFloor);
                    const std::vector<Staircase>& stairs = m_map->getStaircases(m_currentFloor);

                    int spawnX, spawnY;

                    if (!stairs.empty()) {
                        const Staircase& stair = stairs[0];
                        int stairCenterX = stair.bounds.x + stair.bounds.w / 2;
                        int stairCenterY = stair.bounds.y + stair.bounds.h / 2;
                        spawnX = stairCenterX - Player::WIDTH / 2;
                        spawnY = stairCenterY - Player::HEIGHT / 2;
                    }
                    else {
                        spawnX = floorData.corridorRect.x + floorData.corridorRect.w / 2 - Player::WIDTH / 2;
                        spawnY = floorData.floorRect.y + floorData.floorRect.h / 2 - Player::HEIGHT / 2;
                    }

                    bool inRoom = true;
                    int attempts = 0;
                    while (inRoom && attempts < 10) {
                        inRoom = false;
                        for (const auto& checkRoom : rooms) {
                            SDL_Rect playerTest = { spawnX, spawnY, Player::WIDTH, Player::HEIGHT };
                            if (SDL_HasIntersection(&playerTest, &checkRoom.bounds)) {
                                inRoom = true;
                                spawnX = floorData.corridorRect.x + floorData.corridorRect.w / 2 - Player::WIDTH / 2;
                                spawnY = floorData.floorRect.y + floorData.floorRect.h / 2 - Player::HEIGHT / 2;
                                break;
                            }
                        }
                        attempts++;
                    }

                    spawnX = std::max(10, std::min(spawnX, Map::MAP_W - Player::WIDTH - 10));
                    spawnY = std::max(10, std::min(spawnY, Map::MAP_H - Player::HEIGHT - 10));

                    m_player->setPosition(spawnX, spawnY);
                    m_camera->update(m_player->getX() + Player::WIDTH / 2.f,
                        m_player->getY() + Player::HEIGHT / 2.f, 0.016f);
                }
            }
        }
        m_eWasPressed = ePressed;
        return;
    }

    const std::vector<Staircase>& stairs = m_map->getStaircases(m_currentFloor);
    for (size_t i = 0; i < stairs.size(); ++i) {
        const Staircase& stair = stairs[i];
        SDL_Rect exp = { stair.bounds.x - 15, stair.bounds.y - 15,
                         stair.bounds.w + 30, stair.bounds.h + 30 };
        if (Collision::checkAABB(pb, exp)) {
            std::string dir = (stair.toFloor > m_currentFloor) ? "вверх" : "вниз";
            m_interactHint = "[E] Сходи " + dir + " (поверх " + std::to_string(stair.toFloor + 1) + ")";
            if (ePressed && !m_eWasPressed) {
                m_currentFloor = stair.toFloor;
                m_insideRoomIdx = -1;
                m_player->setPosition(stair.spawnX - Player::WIDTH / 2,
                    stair.spawnY - Player::HEIGHT / 2);
                m_camera->x = std::max(0.f, (float)(stair.spawnX - SCREEN_W / 2));
                m_camera->y = std::max(0.f, (float)(stair.spawnY - SCREEN_H / 2));
            }
            m_eWasPressed = ePressed;
            return;
        }
    }
    const std::vector<Room>& rooms = m_map->getRooms(m_currentFloor);
    for (int i = 0; i < (int)rooms.size(); ++i) {
        const Room& room = rooms[i];
        bool leftSideRoom = (room.door.x > room.bounds.x + room.bounds.w / 2);
        SDL_Rect doorExp;

        if (leftSideRoom) {
            doorExp = { room.door.x - 4, room.door.y - 16,
                        room.door.w + 40, room.door.h + 32 };
        }
        else {
            doorExp = { room.door.x - 40, room.door.y - 16,
                        room.door.w + 44, room.door.h + 32 };
        }

        if (Collision::checkAABB(pb, doorExp)) {
            m_interactHint = "[E] Увійти: " + room.label;
            if (ePressed && !m_eWasPressed) {
                m_insideRoomIdx = i;
                int cx = room.bounds.x + room.bounds.w / 2 - Player::WIDTH / 2;
                int cy = room.bounds.y + room.bounds.h / 2 - Player::HEIGHT / 2;
                m_player->setPosition(cx, cy);
                m_camera->update(m_player->getX() + Player::WIDTH / 2.f,
                    m_player->getY() + Player::HEIGHT / 2.f, 0.016f);
            }
            break;
        }
    }

    m_eWasPressed = ePressed;
}

void Game::updateBullets(float dt) {
    const auto& walls = currentWalls();
    for (auto& b : m_bullets) {
        if (!b.active) continue;
        SDL_Rect bRect = { (int)b.x - 3, (int)b.y - 3, 6, 6 };
        for (const auto& w : walls) {
            if (Collision::checkAABB(bRect, w)) { b.active = false; break; }
        }
        if (!b.active) continue;
        for (auto& npc : m_npcs) {
            if (!npc.isAlive() || npc.getFloor() != m_currentFloor) continue;
            auto npcCenterInRoom = [&](const SDL_Rect& rb) -> bool {
                int ncx = npc.getBounds().x + npc.getBounds().w / 2;
                int ncy = npc.getBounds().y + npc.getBounds().h / 2;
                return ncx >= rb.x && ncx < rb.x + rb.w && ncy >= rb.y && ncy < rb.y + rb.h;
            };
            bool npcInSameZone = false;
            if (m_insideRoomIdx >= 0) {
                const auto& room = m_map->getRooms(m_currentFloor)[m_insideRoomIdx];
                if (npcCenterInRoom(room.bounds)) npcInSameZone = true;
            }
            else {
                bool inAnyRoom = false;
                for (const auto& room : m_map->getRooms(m_currentFloor))
                    if (npcCenterInRoom(room.bounds)) { inAnyRoom = true; break; }
                npcInSameZone = !inAnyRoom;
            }
            if (!npcInSameZone) continue;
            if (Collision::checkAABB(bRect, npc.getBounds())) {
                float len = std::sqrt(b.vx * b.vx + b.vy * b.vy);
                float dx = len > 0 ? b.vx / len : 0.f;
                float dy = len > 0 ? b.vy / len : 0.f;
                bool wasAlive = npc.isAlive();
                npc.takeDamage(b.damage, dx, dy);
                if (wasAlive && !npc.isAlive()) m_kills++;
                b.active = false;
                break;
            }
        }
    }
    m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
        [](const Bullet& b) { return !b.active; }), m_bullets.end());
}

const std::vector<SDL_Rect>& Game::currentWalls() const {
    if (m_insideRoomIdx >= 0)
        return m_map->getRoomWalls(m_currentFloor, m_insideRoomIdx);
    return m_map->getWalls(m_currentFloor);
}

void Game::update(float dt) {
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    m_player->handleInput(keys);

    int mx, my; SDL_GetMouseState(&mx, &my);
    float pcx = m_player->getX() + Player::WIDTH / 2.f - m_camera->x;
    float pcy = m_player->getY() + Player::HEIGHT / 2.f - m_camera->y;
    float dx = mx - pcx, dy = my - pcy;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.1f) { m_player->aimDirX = dx / len; m_player->aimDirY = dy / len; }

    m_player->update(dt, currentWalls(), Map::MAP_W, Map::MAP_H);

    
    if (m_insideRoomIdx < 0) {
        const auto& rooms = m_map->getRooms(m_currentFloor);
        SDL_Rect pb = m_player->getBounds();
        for (const auto& room : rooms) {
            if (!SDL_HasIntersection(&pb, &room.bounds)) continue;
 
            bool leftSideRoom = (room.door.x > room.bounds.x + room.bounds.w / 2);
            if (leftSideRoom) {

                int wallX = room.bounds.x + room.bounds.w - 16; 
                if (pb.x < wallX) {

                    m_player->setPosition(wallX, m_player->getY());
                }
            } else {

                int wallX = room.bounds.x + 16; 
                if (pb.x + pb.w > wallX) {
                    m_player->setPosition(wallX - pb.w, m_player->getY());
                }
            }
        }
    }

    m_camera->update(m_player->getX() + Player::WIDTH / 2.f,
        m_player->getY() + Player::HEIGHT / 2.f, dt);

    Uint32 mouseState = SDL_GetMouseState(nullptr, nullptr);
    if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (auto* w = m_inventory->getCurrentWeapon()) {
            bool fired = w->tryFire(m_player->muzzleX(), m_player->muzzleY(),
                m_player->aimDirX, m_player->aimDirY, m_bullets);
            if (fired) m_muzzleFlash = 0.08f;
        }
    }
    if (m_muzzleFlash > 0.f) m_muzzleFlash -= dt;

    m_inventory->update(dt, m_bullets);
    updateBullets(dt);

    float px = m_player->getX() + Player::WIDTH / 2.f;
    float py = m_player->getY() + Player::HEIGHT / 2.f;

    for (auto& npc : m_npcs) {
        if (npc.getFloor() != m_currentFloor) continue;
        auto npcCenterInRoom = [&](const SDL_Rect& roomBounds) -> bool {
            int ncx = npc.getBounds().x + npc.getBounds().w / 2;
            int ncy = npc.getBounds().y + npc.getBounds().h / 2;
            return ncx >= roomBounds.x && ncx < roomBounds.x + roomBounds.w &&
                   ncy >= roomBounds.y && ncy < roomBounds.y + roomBounds.h;
        };
        bool npcInSameZone = false;
        if (m_insideRoomIdx >= 0) {
            const auto& room = m_map->getRooms(m_currentFloor)[m_insideRoomIdx];
            if (npcCenterInRoom(room.bounds)) npcInSameZone = true;
        }
        else {
            bool inAnyRoom = false;
            for (const auto& room : m_map->getRooms(m_currentFloor))
                if (npcCenterInRoom(room.bounds)) { inAnyRoom = true; break; }
            npcInSameZone = !inAnyRoom;
        }
        if (npcInSameZone)
            npc.update(dt, px, py, currentWalls(), Map::MAP_W, Map::MAP_H);

        if (m_insideRoomIdx < 0 && npcInSameZone) {
            const auto& rooms = m_map->getRooms(m_currentFloor);
            for (const auto& room : rooms) {
                if (!npcCenterInRoom(room.bounds)) continue;
                bool leftSideRoom = (room.door.x > room.bounds.x + room.bounds.w / 2);
                if (leftSideRoom) {
                    npc.setPosition((float)(room.bounds.x + room.bounds.w - 16 + 2), npc.getY());
                } else {
                    npc.setPosition((float)(room.bounds.x + 16 - Npc::WIDTH - 2), npc.getY());
                }
                break;
            }
        }
    }

    checkInteractions();
}

void Game::renderBullets() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    for (const auto& b : m_bullets) {
        if (!b.active) continue;
        for (int i = 0; i < (int)b.trail.size(); ++i) {
            float frac = (float)i / (float)b.trail.size();
            SDL_SetRenderDrawColor(m_renderer, 255, (Uint8)(200 * frac), 0, (Uint8)(180 * frac));
            SDL_Rect r = { (int)(b.trail[i].x - m_camera->x) - 1,
                           (int)(b.trail[i].y - m_camera->y) - 1, 3, 3 };
            SDL_RenderFillRect(m_renderer, &r);
        }
        SDL_SetRenderDrawColor(m_renderer, 255, 240, 80, 255);
        SDL_Rect r = { (int)(b.x - m_camera->x) - 3, (int)(b.y - m_camera->y) - 3, 6, 6 };
        SDL_RenderFillRect(m_renderer, &r);
    }

    if (m_muzzleFlash > 0.f) {
        float mx = m_player->muzzleX() - m_camera->x;
        float my = m_player->muzzleY() - m_camera->y;
        float alpha = m_muzzleFlash / 0.08f;
        SDL_SetRenderDrawColor(m_renderer, 255, 200, 50, (Uint8)(200 * alpha));
        for (int r = 16; r >= 4; r -= 4) {
            SDL_Rect fl = { (int)(mx - r),(int)(my - r),r * 2,r * 2 };
            SDL_RenderFillRect(m_renderer, &fl);
        }
    }

    int cmx, cmy; SDL_GetMouseState(&cmx, &cmy);
    SDL_SetRenderDrawColor(m_renderer, 255, 50, 50, 220);
    SDL_RenderDrawLine(m_renderer, cmx - 14, cmy, cmx - 4, cmy);
    SDL_RenderDrawLine(m_renderer, cmx + 4, cmy, cmx + 14, cmy);
    SDL_RenderDrawLine(m_renderer, cmx, cmy - 14, cmx, cmy - 4);
    SDL_RenderDrawLine(m_renderer, cmx, cmy + 4, cmx, cmy + 14);
    SDL_SetRenderDrawColor(m_renderer, 255, 50, 50, 255);
    SDL_Rect cdot = { cmx - 2, cmy - 2, 4, 4 };
    SDL_RenderFillRect(m_renderer, &cdot);
}

void Game::drawHUD() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect hudBg = { 0, 0, SCREEN_W, 38 };
    SDL_SetRenderDrawColor(m_renderer, 10, 15, 35, 190);
    SDL_RenderFillRect(m_renderer, &hudBg);

    std::string location = m_insideRoomIdx >= 0
        ? m_map->getRooms(m_currentFloor)[m_insideRoomIdx].label
        : ("Коридор, поверх " + std::to_string(m_currentFloor + 1));

    std::string info = m_player->getName() + "   " + location +
        "   Вбито: " + std::to_string(m_kills);
    drawText(info, 10, 10, { 220,235,255,255 });

    if (auto* w = m_inventory->getCurrentWeapon()) {
        std::string wInfo = w->getDef().name + "  " +
            std::to_string(w->getMagAmmo()) + "/" + std::to_string(w->getAmmo());
        if (w->isReloading()) wInfo += " [ПЕРЕЗАРЯДКА]";
        SDL_Surface* s = TTF_RenderUTF8_Blended(m_smallFont, wInfo.c_str(), { 255,220,80,255 });
        if (s) {
            int tw = s->w; SDL_FreeSurface(s);
            drawText(wInfo, SCREEN_W - tw - 10, 10, { 255,220,80,255 });
        }
    }

    m_inventory->render(m_renderer, SCREEN_W, SCREEN_H);
    drawText("ESC-пауза  R-перезарядка  1/2/3-зброя",
        SCREEN_W / 2, SCREEN_H - 88, { 120,140,180,200 }, true);

    if (!m_interactHint.empty()) {
        SDL_Rect hintBg = { 0, 40, SCREEN_W, 36 };
        SDL_SetRenderDrawColor(m_renderer, 10, 15, 35, 210);
        SDL_RenderFillRect(m_renderer, &hintBg);
        drawText(m_interactHint, SCREEN_W / 2, 52, { 255,240,100,255 }, true);
    }
}

void Game::render() {
    SDL_ShowCursor(m_state != GameState::Playing ? SDL_ENABLE : SDL_DISABLE);
    SDL_SetRenderDrawColor(m_renderer, 8, 12, 28, 255);
    SDL_RenderClear(m_renderer);

    if (m_state == GameState::Menu) {
        m_menu->render(m_renderer, MenuState::CharacterSelect);
    }
    else {
        m_map->render(m_renderer, *m_camera, m_currentFloor, m_smallFont, m_insideRoomIdx);

        for (auto& npc : m_npcs) {
            if (npc.getFloor() != m_currentFloor) continue;
            auto npcCenterInRoom = [&](const SDL_Rect& rb) -> bool {
                int ncx = npc.getBounds().x + npc.getBounds().w / 2;
                int ncy = npc.getBounds().y + npc.getBounds().h / 2;
                return ncx >= rb.x && ncx < rb.x + rb.w && ncy >= rb.y && ncy < rb.y + rb.h;
            };
            bool npcInSameZone = false;
            if (m_insideRoomIdx >= 0) {
                const auto& room = m_map->getRooms(m_currentFloor)[m_insideRoomIdx];
                npcInSameZone = npcCenterInRoom(room.bounds);
            }
            else {
                bool inAnyRoom = false;
                for (const auto& room : m_map->getRooms(m_currentFloor))
                    if (npcCenterInRoom(room.bounds)) { inAnyRoom = true; break; }
                npcInSameZone = !inAnyRoom;
            }
            if (npcInSameZone) npc.renderBlood(m_renderer, *m_camera);
        }

        for (auto& npc : m_npcs) {
            if (npc.getFloor() != m_currentFloor) continue;
            auto npcCenterInRoom = [&](const SDL_Rect& rb) -> bool {
                int ncx = npc.getBounds().x + npc.getBounds().w / 2;
                int ncy = npc.getBounds().y + npc.getBounds().h / 2;
                return ncx >= rb.x && ncx < rb.x + rb.w && ncy >= rb.y && ncy < rb.y + rb.h;
            };
            bool npcInSameZone = false;
            if (m_insideRoomIdx >= 0) {
                const auto& room = m_map->getRooms(m_currentFloor)[m_insideRoomIdx];
                npcInSameZone = npcCenterInRoom(room.bounds);
            }
            else {
                bool inAnyRoom = false;
                for (const auto& room : m_map->getRooms(m_currentFloor))
                    if (npcCenterInRoom(room.bounds)) { inAnyRoom = true; break; }
                npcInSameZone = !inAnyRoom;
            }
            if (npcInSameZone) npc.render(m_renderer, *m_camera);
        }

        m_player->render(m_renderer, *m_camera);
        renderBullets();
        drawHUD();

        if (m_state == GameState::Paused)
            m_menu->render(m_renderer, MenuState::Pause);
    }

    SDL_RenderPresent(m_renderer);
}

void Game::drawText(const std::string& text, int x, int y, SDL_Color color,
    bool centered, TTF_Font* font) {
    TTF_Font* f = font ? font : m_smallFont;
    if (!f) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(f, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surf);
    int dx = centered ? x - surf->w / 2 : x;
    SDL_Rect dst = { dx, y, surf->w, surf->h };
    SDL_FreeSurface(surf);
    if (tex) { SDL_RenderCopy(m_renderer, tex, nullptr, &dst); SDL_DestroyTexture(tex); }
}

void Game::shutdown() {
    SDL_ShowCursor(SDL_ENABLE);
    TextureManager::getInstance().clear();
    if (m_smallFont) { TTF_CloseFont(m_smallFont); m_smallFont = nullptr; }
    if (m_font) { TTF_CloseFont(m_font);      m_font = nullptr; }
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window) { SDL_DestroyWindow(m_window);     m_window = nullptr; }
    TTF_Quit(); IMG_Quit(); SDL_Quit();
}