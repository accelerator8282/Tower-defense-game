// Tower.cpp — 3 tower types + 3-level upgrade system
#include "Tower.hpp"
#include "Enemy.hpp"
#include <cmath>
#include <algorithm>

// ══════════════════════════════════════════════════════════════════════════════
//  Static helpers
// ══════════════════════════════════════════════════════════════════════════════

int Tower::buyCost(TowerType t) {
    switch (t) {
        case TowerType::Basic:  return 50;
        case TowerType::Sniper: return 75;
        case TowerType::Slow:   return 60;
    }
    return 50;
}

int Tower::upgradeCost(int currentLevel) {
    // Level 1→2: 60, Level 2→3: 100
    return (currentLevel == 1) ? 60 : 100;
}

std::string Tower::typeName(TowerType t) {
    switch (t) {
        case TowerType::Basic:  return "Basic";
        case TowerType::Sniper: return "Sniper";
        case TowerType::Slow:   return "Slow";
    }
    return "?";
}

// ══════════════════════════════════════════════════════════════════════════════
//  Level-style tables
//  Basic  → steel blue  → brighter blue  → gold-outlined white
//  Sniper → dark red    → vivid red      → crimson + thick gold outline
//  Slow   → dark cyan   → vivid cyan     → icy white + thick cyan outline
// ══════════════════════════════════════════════════════════════════════════════

static const LevelStyle BASIC_STYLES[3] = {
    { sf::Color(60,  100, 220), sf::Color(20,  40, 120), 2.f },   // Lv1
    { sf::Color(80,  140, 255), sf::Color(30,  60, 180), 3.f },   // Lv2
    { sf::Color(200, 220, 255), sf::Color(255, 200, 50), 4.f },   // Lv3
};
static const LevelStyle SNIPER_STYLES[3] = {
    { sf::Color(180, 40,  40),  sf::Color(80,  10, 10),  2.f },   // Lv1
    { sf::Color(220, 60,  60),  sf::Color(140, 20, 20),  3.f },   // Lv2
    { sf::Color(255, 80,  80),  sf::Color(255, 200, 30), 5.f },   // Lv3
};
static const LevelStyle SLOW_STYLES[3] = {
    { sf::Color(30,  160, 160), sf::Color(0,   80,  80), 2.f },   // Lv1
    { sf::Color(50,  200, 200), sf::Color(0,  120, 120), 3.f },   // Lv2
    { sf::Color(180, 255, 255), sf::Color(0,  200, 220), 5.f },   // Lv3
};

// ══════════════════════════════════════════════════════════════════════════════
//  Constructor
// ══════════════════════════════════════════════════════════════════════════════

Tower::Tower(sf::Vector2f pos, TowerType t)
    : Entity(pos), type(t)
{
    // Base stats by type
    switch (type) {
        case TowerType::Basic:
            range      = 120.f;
            fireRate   = 1.5f;
            damage     = 15;
            slowFactor = 0.f;
            break;
        case TowerType::Sniper:
            range      = 160.f;   // longer reach
            fireRate   = 0.8f;    // slower
            damage     = 30;      // ~2× damage
            slowFactor = 0.f;
            break;
        case TowerType::Slow:
            range      = 110.f;
            fireRate   = 1.2f;
            damage     = 8;       // low dmg — utility tower
            slowFactor = 0.25f;   // 25% speed reduction on hit
            break;
    }

    // Shape: Basic/Slow = hexagon (6pts), Sniper = octagon (8pts)
    int pts = (type == TowerType::Sniper) ? 8 : 6;
    auto body = std::make_unique<sf::CircleShape>(15.f, pts);
    body->setOrigin(sf::Vector2f(15.f, 15.f));
    body->setPosition(position);
    bodyPtr = body.get();
    shape   = std::move(body);

    applyLevelStyle();   // sets fill/outline colors for level 1

    // Range indicator
    rangeShape = std::make_unique<sf::CircleShape>(range);
    rangeShape->setFillColor(sf::Color(100, 160, 255, 25));
    rangeShape->setOutlineColor(sf::Color(160, 200, 255, 100));
    rangeShape->setOutlineThickness(1.f);
    rangeShape->setOrigin(sf::Vector2f(range, range));
    rangeShape->setPosition(position);
    rangePtr = rangeShape.get();
}

// ══════════════════════════════════════════════════════════════════════════════
//  applyLevelStyle — sets body fill/outline from style table
// ══════════════════════════════════════════════════════════════════════════════

void Tower::applyLevelStyle() {
    int idx = std::min(level - 1, 2);
    const LevelStyle* styles = nullptr;
    switch (type) {
        case TowerType::Basic:  styles = BASIC_STYLES;  break;
        case TowerType::Sniper: styles = SNIPER_STYLES; break;
        case TowerType::Slow:   styles = SLOW_STYLES;   break;
    }
    if (!styles) return;
    bodyPtr->setFillColor(styles[idx].fill);
    bodyPtr->setOutlineColor(styles[idx].outline);
    bodyPtr->setOutlineThickness(styles[idx].outlineThick);
}

// ══════════════════════════════════════════════════════════════════════════════
//  upgrade
// ══════════════════════════════════════════════════════════════════════════════

void Tower::upgrade() {
    if (level >= MAX_LEVEL) return;
    ++level;

    switch (type) {
        case TowerType::Basic:
            damage   += 8;
            fireRate += 0.6f;
            range    += 15.f;
            break;
        case TowerType::Sniper:
            damage   += 15;
            fireRate += 0.3f;
            range    += 25.f;
            break;
        case TowerType::Slow:
            damage     += 4;
            fireRate   += 0.4f;
            slowFactor  = std::min(0.5f, slowFactor + 0.1f);  // up to 50% slow
            range      += 10.f;
            break;
    }

    applyLevelStyle();

    // Resize range indicator
    rangePtr->setRadius(range);
    rangePtr->setOrigin(sf::Vector2f(range, range));
}

// ══════════════════════════════════════════════════════════════════════════════
//  Targeting & shooting
// ══════════════════════════════════════════════════════════════════════════════

Enemy* Tower::findTarget(const std::vector<Enemy*>& enemies) const {
    Enemy* best     = nullptr;
    float  bestDist = range + 1.f;
    for (Enemy* e : enemies) {
        if (!e || !e->isAlive()) continue;
        float dx = e->getPosition().x - position.x;
        float dy = e->getPosition().y - position.y;
        float d  = std::sqrt(dx*dx + dy*dy);
        if (d <= range && d < bestDist) { bestDist = d; best = e; }
    }
    return best;
}

bool Tower::canShoot() const noexcept {
    return fireCooldown <= 0.f && target && target->isAlive();
}

sf::Vector2f Tower::shoot() {
    fireCooldown = 1.f / fireRate;
    sf::Vector2f dir = target->getPosition() - position;
    float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len > 0.f) dir /= len;
    return dir;
}

void Tower::update(float dt, const std::vector<Enemy*>& enemies) {
    if (target && !target->isAlive()) target = nullptr;
    target = findTarget(enemies);
    if (fireCooldown > 0.f) fireCooldown -= dt;
}

// ══════════════════════════════════════════════════════════════════════════════
//  render
// ══════════════════════════════════════════════════════════════════════════════

void Tower::render(sf::RenderWindow& win) {
    if (showRange) win.draw(*rangePtr);

    win.draw(*bodyPtr);

    // Level badge: small number circle in top-right corner of tower
    if (level > 1) {
        float badgeR = 7.f;
        sf::CircleShape badge(badgeR);
        sf::Color badgeCol = (level == 2) ? sf::Color(255, 200, 0)
                                           : sf::Color(255, 80, 200);
        badge.setFillColor(badgeCol);
        badge.setOutlineColor(sf::Color(20, 20, 20));
        badge.setOutlineThickness(1.f);
        badge.setOrigin(sf::Vector2f(badgeR, badgeR));
        badge.setPosition(position + sf::Vector2f(11.f, -11.f));
        win.draw(badge);
    }

    // Barrel toward target
    if (target && target->isAlive()) {
        sf::Vector2f dir = target->getPosition() - position;
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (len > 0.f) {
            dir /= len;
            sf::Color barrelCol;
            switch (type) {
                case TowerType::Basic:  barrelCol = sf::Color(180, 220, 255); break;
                case TowerType::Sniper: barrelCol = sf::Color(255, 160, 160); break;
                case TowerType::Slow:   barrelCol = sf::Color(100, 255, 255); break;
            }
            sf::Vertex barrel[2];
            barrel[0].position = position;
            barrel[0].color    = barrelCol;
            barrel[1].position = position + dir * 22.f;
            barrel[1].color    = sf::Color::White;
            win.draw(barrel, 2, sf::PrimitiveType::Lines);
        }
    }
}
