#include "menu.hpp"
#include "texturemanager.hpp"
#include <algorithm>

std::vector<CharacterDef> Menu::buildCharacters() {
    return {
        { CharacterType::Janna,    "Жанна (Учитель)",   "char_janna",    "janna.png"    },
        { CharacterType::Vorona,   "Ворона (Учитель)",  "char_vorona",   "vorona.png"   },
        { CharacterType::Popovich, "Попович (Учень)",   "char_popovich", "popovich.png" },
        { CharacterType::Char4,    "Супрун (Учень)",            "char_4",        "zlagod.png"   },
        { CharacterType::Char5,    "Бездрапко (Учень)",     "char_5",        "bayraktar.png"},
        { CharacterType::Char6,    "Плескач (Учень)",              "char_6",        "senya.png"    },
    };
}

Menu::Menu() {
    m_characters = buildCharacters();
    m_scrollOffset = 0;
    m_maxScroll = std::max(0, (int)m_characters.size() * (CARD_W + CARD_GAP) - 800 + 100);
}

Menu::~Menu() {}

void Menu::init(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* smallFont) {
    m_renderer = renderer;
    m_font = font;
    m_smallFont = smallFont;
}

void Menu::handleEvent(const SDL_Event& e) {
    int totalW = (int)m_characters.size() * (CARD_W + CARD_GAP);
    int startX = (800 - std::min(totalW, 800)) / 2 - m_scrollOffset;

    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_a) {
            m_hovered = std::max(0, m_hovered - 1);
            int cardX = m_hovered * (CARD_W + CARD_GAP);
            if (cardX - m_scrollOffset < 50) {
                m_scrollOffset = std::max(0, cardX - 50);
            }
            if (m_hovered == 0) m_scrollOffset = 0;
        }
        if (e.key.keysym.sym == SDLK_RIGHT || e.key.keysym.sym == SDLK_d) {
            m_hovered = std::min((int)m_characters.size() - 1, m_hovered + 1);
            int cardX = m_hovered * (CARD_W + CARD_GAP);
            if (cardX - m_scrollOffset > 800 - CARD_W - 50) {
                m_scrollOffset = std::min(m_maxScroll, cardX - 800 + CARD_W + 50);
            }
        }
        if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_SPACE)
            m_selected = m_hovered;
    }
    if (e.type == SDL_MOUSEMOTION) {
        int mx = e.motion.x;
        for (int i = 0; i < (int)m_characters.size(); ++i) {
            int cx = startX + i * (CARD_W + CARD_GAP);
            if (mx >= cx && mx <= cx + CARD_W) {
                m_hovered = i;
                break;
            }
        }
    }
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = e.button.x, my = e.button.y;
        for (int i = 0; i < (int)m_characters.size(); ++i) {
            int cx = startX + i * (CARD_W + CARD_GAP);
            if (mx >= cx && mx <= cx + CARD_W && my >= 200 && my <= 200 + CARD_H) {
                m_selected = i; break;
            }
        }
    }
    if (e.type == SDL_MOUSEWHEEL) {
        m_scrollOffset -= e.wheel.y * 40;
        m_scrollOffset = std::max(0, std::min(m_maxScroll, m_scrollOffset));
        int totalW = (int)m_characters.size() * (CARD_W + CARD_GAP);
        int startX = (800 - std::min(totalW, 800)) / 2 - m_scrollOffset;
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        for (int i = 0; i < (int)m_characters.size(); ++i) {
            int cx = startX + i * (CARD_W + CARD_GAP);
            if (mx >= cx && mx <= cx + CARD_W && my >= 200 && my <= 200 + CARD_H) {
                m_hovered = i; break;
            }
        }
    }
}

void Menu::drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
    int x, int y, SDL_Color color, bool centered) {
    if (!font) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    int dx = centered ? x - surf->w / 2 : x;
    SDL_Rect dst = { dx, y, surf->w, surf->h };
    SDL_FreeSurface(surf);
    if (tex) { SDL_RenderCopy(renderer, tex, nullptr, &dst); SDL_DestroyTexture(tex); }
}

void Menu::drawCard(SDL_Renderer* renderer, const CharacterDef& def, SDL_Rect area, bool hovered) {
    SDL_Color bg = hovered ? SDL_Color{ 60,80,130,230 } : SDL_Color{ 30,40,70,200 };
    SDL_Color border = hovered ? SDL_Color{ 200,220,255,255 } : SDL_Color{ 80,100,160,255 };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(renderer, &area);
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(renderer, &area);

    SDL_Rect imgArea = { area.x + 15, area.y + 15, area.w - 30, area.h - 70 };
    SDL_Texture* tex = TextureManager::getInstance().get(def.textureId);
    if (tex) {
        SDL_Rect src = { 0,0,0,0 };
        SDL_QueryTexture(tex, nullptr, nullptr, &src.w, &src.h);
        float scale = std::min((float)imgArea.w / src.w, (float)imgArea.h / src.h);
        int newW = (int)(src.w * scale);
        int newH = (int)(src.h * scale);
        SDL_Rect dst = { imgArea.x + (imgArea.w - newW) / 2, imgArea.y + (imgArea.h - newH) / 2, newW, newH };
        SDL_RenderCopy(renderer, tex, &src, &dst);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 60, 100, 180, 200);
        SDL_RenderFillRect(renderer, &imgArea);
        int cx = imgArea.x + imgArea.w / 2, cy = imgArea.y + imgArea.h / 3;
        SDL_Rect head = { cx - 12, cy - 14, 24, 24 };
        SDL_SetRenderDrawColor(renderer, 200, 160, 120, 255);
        SDL_RenderFillRect(renderer, &head);
        SDL_Rect body = { cx - 14, cy + 12, 28, 36 };
        SDL_SetRenderDrawColor(renderer, 80, 130, 200, 255);
        SDL_RenderFillRect(renderer, &body);
    }
    SDL_Color textCol = hovered ? SDL_Color{ 255,255,255,255 } : SDL_Color{ 180,200,230,255 };
    drawText(renderer, m_smallFont, def.name,
        area.x + area.w / 2, area.y + area.h - 52, textCol, true);
}

void Menu::renderCharacterSelect(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 10, 15, 35, 255);
    SDL_RenderClear(renderer);
    for (int i = 0; i < 20; ++i) {
        SDL_SetRenderDrawColor(renderer, 25 + i, 35 + i, 70 + i * 2, 40);
        SDL_Rect strip = { 0, i * 32, 800, 2 };
        SDL_RenderFillRect(renderer, &strip);
    }

    drawText(renderer, m_font, "ПКНГ SHOOTER", 400, 40, { 220,235,255,255 }, true);
    drawText(renderer, m_smallFont, "Оберіть персонажа", 400, 100, { 160,180,220,200 }, true);

    if (m_scrollOffset > 0) {
        drawText(renderer, m_font, "<<", 30, 320, { 255,255,255,200 }, true);
    }
    if (m_scrollOffset < m_maxScroll) {
        drawText(renderer, m_font, ">>", 770, 320, { 255,255,255,200 }, true);
    }

    drawText(renderer, m_smallFont, "WASD / Стрілки / Миша - навігація, Enter - вибір",
        400, 150, { 120,140,180,180 }, true);
    drawText(renderer, m_smallFont, "Колесо миші - гортання", 400, 170, { 100,120,150,180 }, true);

    int totalW = (int)m_characters.size() * (CARD_W + CARD_GAP);
    int startX = (800 - std::min(totalW, 800)) / 2 - m_scrollOffset;
    int startY = 210;

    SDL_Rect clip = { 0, 0, 800, 600 };
    SDL_RenderSetClipRect(renderer, &clip);

    for (int i = 0; i < (int)m_characters.size(); ++i) {
        SDL_Rect area = { startX + i * (CARD_W + CARD_GAP), startY, CARD_W, CARD_H };
        if (area.x + area.w > 0 && area.x < 800) {
            drawCard(renderer, m_characters[i], area, i == m_hovered);
        }
    }

    SDL_RenderSetClipRect(renderer, nullptr);

    drawText(renderer, m_smallFont,
        "WASD-рух  ЛКМ-стріляти  E/Enter-двері  R-перезарядка  1/2/3-зброя  ESC-пауза",
        400, 560, { 120,200,120,255 }, true);
}

void Menu::renderPause(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect overlay = { 0,0,800,600 };
    SDL_RenderFillRect(renderer, &overlay);
    SDL_Rect panel = { 250,180,300,240 };
    SDL_SetRenderDrawColor(renderer, 20, 30, 60, 230);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 100, 130, 200, 255);
    SDL_RenderDrawRect(renderer, &panel);
    drawText(renderer, m_font, "ПАУЗА", 400, 210, { 220,235,255,255 }, true);
    drawText(renderer, m_smallFont, "ESC - продовжити", 400, 300, { 180,200,230,255 }, true);
    drawText(renderer, m_smallFont, "F1 - повернутись в меню", 400, 340, { 180,200,230,255 }, true);
}

void Menu::render(SDL_Renderer* renderer, MenuState state) {
    if (state == MenuState::CharacterSelect) renderCharacterSelect(renderer);
    else if (state == MenuState::Pause)      renderPause(renderer);
}