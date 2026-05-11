// Tower.hpp — 3 tower types + working upgrade system
#pragma once
#include "TowerType.hpp"
#include "Entity.hpp"
#include <vector>
#include <memory>
#include <string>

class Enemy;

// ── Per-upgrade-level visuals ───────────────────────────────────────────────
struct LevelStyle {
    sf::Color  fill;
    sf::Color  outline;
    float      outlineThick;
};

class Tower : public Entity {
public:
    Tower(sf::Vector2f pos, TowerType type);
    ~Tower() override = default;

    // Upgrade: up to MAX_LEVEL. Each level buffs stats + changes visuals.
    static constexpr int MAX_LEVEL = 3;
    void upgrade();

    // Called by CombatSystem each frame.
    void update(float dt, const std::vector<Enemy*>& enemies);

    bool         canShoot()      const noexcept;
    sf::Vector2f shoot();           // resets cooldown, returns norm dir

    // Accessors
    TowerType    getType()          const noexcept { return type;          }
    int          getDamage()        const noexcept { return damage;        }
    float        getProjectileSpd() const noexcept { return projectileSpd; }
    int          getLevel()         const noexcept { return level;         }
    float        getRange()         const noexcept { return range;         }
    bool         isShowingRange()   const noexcept { return showRange;     }
    void         setShowRange(bool v)     noexcept { showRange = v;        }

    // Slow tower: returns slow factor (0 = no slow, 0.25 = 25% slower)
    float        getSlowFactor()    const noexcept { return slowFactor;    }
    bool         isSlow()           const noexcept { return slowFactor > 0.f; }

    // Cost to buy / upgrade
    static int buyCost(TowerType t);
    static int upgradeCost(int currentLevel);

    // Human-readable label for HUD
    static std::string typeName(TowerType t);

    // Entity interface
    void  update(float /*dt*/) override {}   // two-arg version is used
    void  render(sf::RenderWindow& win) override;
    float getRadius() const noexcept override { return 15.f; }

private:
    TowerType type;
    float  range;
    float  fireRate;
    float  fireCooldown  = 0.f;
    int    damage;
    int    level         = 1;
    float  projectileSpd = 700.f;
    float  slowFactor    = 0.f;   // non-zero only for Slow tower
    bool   showRange     = false;

    Enemy* target = nullptr;

    sf::CircleShape*                 bodyPtr  = nullptr;
    sf::CircleShape*                 rangePtr = nullptr;
    std::unique_ptr<sf::CircleShape> rangeShape;

    // Per-type per-level style table — set in applyLevelStyle()
    void applyLevelStyle();

    // Returns the nearest living enemy in range
    Enemy* findTarget(const std::vector<Enemy*>& enemies) const;
};
