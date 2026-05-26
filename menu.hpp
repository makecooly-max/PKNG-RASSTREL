#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include "player.hpp"

enum class MenuState { CharacterSelect, Pause };

class Menu {
public:
    static constexpr int CARD_W = 160;
    static constexpr int CARD_H = 240;
    static constexpr int CARD_GAP = 20;

    Menu();
    ~Menu();

    void init(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* smallFont);
    void handleEvent(const SDL_Event& e);
    void render(SDL_Renderer* renderer, MenuState state);

    bool hasSelectedCharacter()   const { return m_selected >= 0; }
    int  getSelectedIndex()       const { return m_selected; }
    const CharacterDef& getSelectedCharacter() const { return m_characters[m_selected]; }
    void resetSelection() { m_selected = -1; m_hovered = 0; m_scrollOffset = 0; }

    static std::vector<CharacterDef> buildCharacters();

private:
    TTF_Font* m_font = nullptr;
    TTF_Font* m_smallFont = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    std::vector<CharacterDef> m_characters;
    int m_hovered = 0;
    int m_selected = -1;
    int m_scrollOffset = 0;
    int m_maxScroll = 0;

    void renderCharacterSelect(SDL_Renderer* renderer);
    void renderPause(SDL_Renderer* renderer);
    void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
        int x, int y, SDL_Color color, bool centered = false);
    void drawCard(SDL_Renderer* renderer, const CharacterDef& def, SDL_Rect area, bool hovered);
};