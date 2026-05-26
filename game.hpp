#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <memory>
#include <string>
#include <vector>
#include "weapon.hpp"
#include "npc.hpp"

enum class GameState { Menu, Playing, Paused };

class Game {
public:
    static constexpr int SCREEN_W = 800;
    static constexpr int SCREEN_H = 600;
    static constexpr int TARGET_FPS = 60;

    Game();
    ~Game();

    bool init();
    void run();
    void shutdown();

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    TTF_Font* m_font = nullptr;
    TTF_Font* m_smallFont = nullptr;

    GameState m_state = GameState::Menu;
    bool      m_running = false;

    std::unique_ptr<class Map>       m_map;
    std::unique_ptr<class Player>    m_player;
    std::unique_ptr<class Camera>    m_camera;
    std::unique_ptr<class Menu>      m_menu;
    std::unique_ptr<class Inventory> m_inventory;

    std::vector<Npc>    m_npcs;
    std::vector<Bullet> m_bullets;

    int  m_currentFloor = 0;
    int  m_insideRoomIdx = -1;
    bool m_eWasPressed = false;
    bool m_f1WasPressed = false;
    bool m_rWasPressed = false;

    std::string m_interactHint;

    float m_muzzleFlash = 0.f;

    int m_kills = 0;

    void handleEvents();
    void update(float dt);
    void render();
    void loadTextures();
    void startGame();
    void spawnNpcs();
    void checkInteractions();
    void updateBullets(float dt);
    void drawHUD();
    void renderBullets();
    void drawText(const std::string& text, int x, int y, SDL_Color color,
        bool centered = false, TTF_Font* font = nullptr);
    const std::vector<SDL_Rect>& currentWalls() const;
};
