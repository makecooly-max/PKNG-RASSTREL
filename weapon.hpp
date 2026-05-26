#pragma once
#include <SDL.h>
#include <string>
#include <vector>

struct Bullet {
    float x, y;
    float vx, vy;
    float life;
    int   damage;
    bool  active;
    struct TrailPt { float x, y; };
    std::vector<TrailPt> trail;
};

enum class WeaponType { Pistol, Shotgun, Rifle };

struct WeaponDef {
    WeaponType  type;
    std::string name;
    int         damage;
    float       fireRate;
    int         bulletsPerShot;
    float       spread;
    float       bulletSpeed;
    float       bulletLife;
    int         magSize;
    float       reloadTime;
};

class Weapon {
public:
    explicit Weapon(const WeaponDef& def);

    bool tryFire(float ox, float oy, float dirX, float dirY,
        std::vector<Bullet>& bullets);
    void reload();
    void update(float dt, std::vector<Bullet>& bullets);

    const WeaponDef& getDef()   const { return m_def; }
    int  getAmmo()              const { return m_ammo; }
    int  getMagAmmo()           const { return m_magAmmo; }
    bool isReloading()          const { return m_reloadTimer > 0.f; }
    float getReloadProgress()   const;
    float getFireCooldown()     const { return m_fireCooldown; }

private:
    WeaponDef m_def;
    int   m_ammo;
    int   m_magAmmo;
    float m_fireCooldown = 0.f;
    float m_reloadTimer = 0.f;
};

namespace Weapons {
    WeaponDef makePistol();
    WeaponDef makeShotgun();
    WeaponDef makeRifle();
}
