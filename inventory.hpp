#pragma once
#include "weapon.hpp"
#include <vector>
#include <memory>

class Inventory {
public:
    Inventory();

    void addWeapon(WeaponDef def);
    void selectNext();
    void selectPrev();
    void selectSlot(int slot);

    Weapon* getCurrentWeapon();
    int     getCurrentSlot()  const { return m_current; }
    int     getCount()        const { return (int)m_weapons.size(); }
    const Weapon& getWeapon(int i) const { return m_weapons[i]; }

    void update(float dt, std::vector<Bullet>& bullets);
    void render(SDL_Renderer* renderer, int screenW, int screenH) const;

private:
    std::vector<Weapon> m_weapons;
    int m_current = 0;
};
