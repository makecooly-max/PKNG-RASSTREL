#include "inventory.hpp"
#include <SDL_ttf.h>
#include <algorithm>

Inventory::Inventory() {
    addWeapon(Weapons::makePistol());
    addWeapon(Weapons::makeShotgun());
    addWeapon(Weapons::makeRifle());
}

void Inventory::addWeapon(WeaponDef def) {
    m_weapons.emplace_back(def);
}

void Inventory::selectNext() {
    if (!m_weapons.empty())
        m_current = (m_current + 1) % (int)m_weapons.size();
}

void Inventory::selectPrev() {
    if (!m_weapons.empty())
        m_current = (m_current - 1 + (int)m_weapons.size()) % (int)m_weapons.size();
}

void Inventory::selectSlot(int slot) {
    if (slot >= 0 && slot < (int)m_weapons.size())
        m_current = slot;
}

Weapon* Inventory::getCurrentWeapon() {
    if (m_weapons.empty()) return nullptr;
    return &m_weapons[m_current];
}

void Inventory::update(float dt, std::vector<Bullet>& bullets) {
    if (!m_weapons.empty())
        m_weapons[m_current].update(dt, bullets);
}

void Inventory::render(SDL_Renderer* renderer, int screenW, int screenH) const {
    if (m_weapons.empty()) return;

    const int slotW = 90, slotH = 60, gap = 8;
    int totalW = (int)m_weapons.size() * slotW + ((int)m_weapons.size() - 1) * gap;
    int startX = screenW / 2 - totalW / 2;
    int startY = screenH - slotH - 12;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < (int)m_weapons.size(); ++i) {
        const Weapon& w = m_weapons[i];
        bool sel = (i == m_current);
        int sx = startX + i * (slotW + gap);

        SDL_SetRenderDrawColor(renderer, sel ? 50 : 20, sel ? 70 : 30, sel ? 120 : 50,
            sel ? 230 : 160);
        SDL_Rect bg = { sx, startY, slotW, slotH };
        SDL_RenderFillRect(renderer, &bg);

        SDL_SetRenderDrawColor(renderer, sel ? 220 : 90, sel ? 240 : 110, 255, 255);
        SDL_RenderDrawRect(renderer, &bg);

        int ammo = w.getMagAmmo();
        int maxAmmo = w.getDef().magSize;
        int dotW = std::max(2, (slotW - 8) / std::max(1, maxAmmo) - 1);
        for (int a = 0; a < maxAmmo; ++a) {
            SDL_SetRenderDrawColor(renderer, a < ammo ? 100 : 50,
                a < ammo ? 220 : 70,
                a < ammo ? 100 : 50, 255);
            SDL_Rect dot = { sx + 4 + a * (dotW + 1), startY + slotH - 10, dotW, 6 };
            SDL_RenderFillRect(renderer, &dot);
        }

        // weapon icon (colored rectangle)
        SDL_Color wCol;
        switch (w.getDef().type) {
        case WeaponType::Pistol:  wCol = { 200,200, 80,255 }; break;
        case WeaponType::Shotgun: wCol = { 220,120, 60,255 }; break;
        case WeaponType::Rifle:   wCol = { 80,200,220,255 }; break;
        default:                  wCol = { 200,200,200,255 }; break;
        }
        SDL_SetRenderDrawColor(renderer, wCol.r, wCol.g, wCol.b, 200);
        SDL_Rect icon;
        switch (w.getDef().type) {
        case WeaponType::Pistol:
            icon = { sx + 18, startY + 16, 24, 10 };
            SDL_RenderFillRect(renderer, &icon);
            { SDL_Rect grip = { sx + 22, startY + 24, 8, 12 }; SDL_RenderFillRect(renderer, &grip); }
            break;
        case WeaponType::Shotgun:
            icon = { sx + 10, startY + 18, 40, 8 };
            SDL_RenderFillRect(renderer, &icon);
            { SDL_Rect barrel = { sx + 8, startY + 16, 10, 12 }; SDL_RenderFillRect(renderer, &barrel); }
            break;
        case WeaponType::Rifle:
            icon = { sx + 8, startY + 19, 50, 7 };
            SDL_RenderFillRect(renderer, &icon);
            { SDL_Rect scope = { sx + 22, startY + 13, 14, 8 }; SDL_RenderFillRect(renderer, &scope); }
            break;
        }

        // reload bar
        if (w.isReloading()) {
            float prog = w.getReloadProgress();
            SDL_SetRenderDrawColor(renderer, 40, 40, 40, 200);
            SDL_Rect rbar = { sx + 4, startY + 4, slotW - 8, 5 };
            SDL_RenderFillRect(renderer, &rbar);
            SDL_SetRenderDrawColor(renderer, 255, 200, 50, 255);
            SDL_Rect rfill = { sx + 4, startY + 4, (int)((slotW - 8) * prog), 5 };
            SDL_RenderFillRect(renderer, &rfill);
        }
    }
}
