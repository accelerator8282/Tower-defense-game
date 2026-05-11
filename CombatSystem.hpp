// CombatSystem.hpp
#pragma once
#include "Tower.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"
#include <vector>
#include <memory>
#include <functional>

class CombatSystem {
public:
    std::function<void(float reward)> onEnemyKilled;
    std::function<void()>             onEnemyEscaped;

    void spawnEnemy(std::unique_ptr<Enemy> e);
    void placeTower(std::unique_ptr<Tower> t);

    Tower* getTowerAt(sf::Vector2f pos, float radius = 40.f);
    bool   upgradeTower(Tower* t, int& outCost);

    // Resets showRange on all towers (call each frame before hover detection)
    void resetAllRangeDisplay();

    void update(float dt);
    void render(sf::RenderWindow& win) const;

    bool hasLivingEnemies() const noexcept;
    std::vector<Enemy*> getLivingEnemyPtrs() const;

private:
    std::vector<std::unique_ptr<Enemy>>      enemies;
    std::vector<std::unique_ptr<Tower>>      towers;
    std::vector<std::unique_ptr<Projectile>> projectiles;

    void purgeDeadEnemies();
    void purgeDeadProjectiles();
};
