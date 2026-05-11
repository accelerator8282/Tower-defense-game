// CombatSystem.cpp
#include "CombatSystem.hpp"
#include <algorithm>
#include <cmath>

void CombatSystem::spawnEnemy(std::unique_ptr<Enemy> e) {
    enemies.push_back(std::move(e));
}

void CombatSystem::placeTower(std::unique_ptr<Tower> t) {
    towers.push_back(std::move(t));
}

Tower* CombatSystem::getTowerAt(sf::Vector2f pos, float radius) {
    for (auto& t : towers) {
        float dx = t->getPosition().x - pos.x;
        float dy = t->getPosition().y - pos.y;
        if (std::sqrt(dx*dx + dy*dy) <= radius)
            return t.get();
    }
    return nullptr;
}

bool CombatSystem::upgradeTower(Tower* t, int& outCost) {
    if (!t || t->getLevel() >= Tower::MAX_LEVEL) return false;
    outCost = Tower::upgradeCost(t->getLevel());
    t->upgrade();
    return true;
}

void CombatSystem::resetAllRangeDisplay() {
    for (auto& t : towers)
        t->setShowRange(false);
}

void CombatSystem::update(float dt) {
    std::vector<Enemy*> living = getLivingEnemyPtrs();

    // 1. Update towers + fire projectiles
    for (auto& tower : towers) {
        tower->update(dt, living);
        if (tower->canShoot()) {
            sf::Vector2f dir = tower->shoot();
            projectiles.push_back(std::make_unique<Projectile>(
                tower->getPosition(), dir,
                tower->getProjectileSpd(),
                tower->getDamage(),
                tower->getSlowFactor(),
                tower->getType()
            ));
        }
    }

    // 2. Update enemies — collect escaped before killing them
    for (auto& e : enemies) {
        e->update(dt);
        if (e->isAlive() && e->reachedEnd()) {
            e->kill();  // mark dead
            if (onEnemyEscaped) onEnemyEscaped();
        }
    }

    // 3. Projectile → enemy collision
    for (auto& proj : projectiles) {
        proj->update(dt);
        if (!proj->isAlive()) continue;

        const float projR = proj->getRadius();
        for (auto& e : enemies) {
            if (!e->isAlive()) continue;
            float dx = proj->getPosition().x - e->getPosition().x;
            float dy = proj->getPosition().y - e->getPosition().y;
            float radSum = projR + e->getRadius();
            if (dx*dx + dy*dy < radSum*radSum) {
                if (proj->getSlowFactor() > 0.f)
                    e->applySlow(proj->getSlowFactor());

                e->takeDamage(static_cast<float>(proj->getDamage()));
                proj->kill();

                if (!e->isAlive() && e->wasKilledByTower())
                    if (onEnemyKilled) onEnemyKilled(e->getReward());
                break;
            }
        }
    }

    // 4. Purge dead entities BEFORE hasLivingEnemies is evaluated
    purgeDeadEnemies();
    purgeDeadProjectiles();
}

void CombatSystem::render(sf::RenderWindow& win) const {
    for (const auto& t : towers)      t->render(win);
    for (const auto& e : enemies)     e->render(win);
    for (const auto& p : projectiles) p->render(win);
}

bool CombatSystem::hasLivingEnemies() const noexcept {
    // After purgeDeadEnemies(), the vector only holds living ones
    return !enemies.empty();
}

std::vector<Enemy*> CombatSystem::getLivingEnemyPtrs() const {
    std::vector<Enemy*> out;
    out.reserve(enemies.size());
    for (const auto& e : enemies)
        if (e->isAlive()) out.push_back(e.get());
    return out;
}

void CombatSystem::purgeDeadEnemies() {
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const std::unique_ptr<Enemy>& e){ return !e->isAlive(); }),
        enemies.end());
}

void CombatSystem::purgeDeadProjectiles() {
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
        [](const std::unique_ptr<Projectile>& p){ return !p->isAlive(); }),
        projectiles.end());
}
