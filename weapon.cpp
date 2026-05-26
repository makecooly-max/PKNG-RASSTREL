#include "weapon.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

WeaponDef Weapons::makePistol() {
    return { WeaponType::Pistol, "Пістолет", 25, 3.f, 1, 0.04f, 700.f, 1.4f, 12, 1.5f };
}
WeaponDef Weapons::makeShotgun() {
    return { WeaponType::Shotgun, "Дробовик", 18, 1.2f, 7, 0.22f, 480.f, 0.7f, 6, 2.5f };
}
WeaponDef Weapons::makeRifle() {
    return { WeaponType::Rifle, "Рушниця", 40, 1.5f, 1, 0.02f, 900.f, 1.8f, 20, 2.0f };
}

Weapon::Weapon(const WeaponDef& def)
    : m_def(def), m_ammo(def.magSize * 5), m_magAmmo(def.magSize)
{
}

bool Weapon::tryFire(float ox, float oy, float dirX, float dirY, std::vector<Bullet>& bullets) {
    if (m_fireCooldown > 0.f || isReloading()) return false;
    if (m_magAmmo <= 0) { reload(); return false; }

    m_fireCooldown = 1.f / m_def.fireRate;
    m_magAmmo--;

    for (int i = 0; i < m_def.bulletsPerShot; ++i) {
        float angle = std::atan2(dirY, dirX);
        float sp = m_def.spread;
        angle += ((rand() % 2000) - 1000) / 1000.f * sp;

        Bullet b;
        b.x = ox; b.y = oy;
        b.vx = std::cos(angle) * m_def.bulletSpeed;
        b.vy = std::sin(angle) * m_def.bulletSpeed;
        b.life = m_def.bulletLife;
        b.damage = m_def.damage;
        b.active = true;
        bullets.push_back(std::move(b));
    }
    return true;
}

void Weapon::reload() {
    if (isReloading() || m_ammo <= 0 || m_magAmmo == m_def.magSize) return;
    m_reloadTimer = m_def.reloadTime;
}

void Weapon::update(float dt, std::vector<Bullet>& bullets) {
    if (m_fireCooldown > 0.f) m_fireCooldown -= dt;
    if (m_reloadTimer > 0.f) {
        m_reloadTimer -= dt;
        if (m_reloadTimer <= 0.f) {
            int needed = m_def.magSize - m_magAmmo;
            int take = std::min(needed, m_ammo);
            m_magAmmo += take;
            m_ammo -= take;
            m_reloadTimer = 0.f;
        }
    }

    for (auto& b : bullets) {
        if (!b.active) continue;
        b.trail.push_back({ b.x, b.y });
        if ((int)b.trail.size() > 6) b.trail.erase(b.trail.begin());
        b.x += b.vx * dt;
        b.y += b.vy * dt;
        b.life -= dt;
        if (b.life <= 0.f) b.active = false;
    }
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const Bullet& b) { return !b.active; }), bullets.end());
}

float Weapon::getReloadProgress() const {
    if (m_def.reloadTime <= 0.f) return 1.f;
    return 1.f - (m_reloadTimer / m_def.reloadTime);
}
