#define SDL_MAIN_HANDLED

#include <Windows.h>
#include <SDL.h>

#include "game.hpp"

int WINAPI WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrev,
    LPSTR lpCmdLine,
    int nShowCmd)
{
    Game game;

    if (!game.init()) {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Помилка",
            "Не вдалося ініціалізувати гру.",
            nullptr
        );

        return 1;
    }

    game.run();

    return 0;
}