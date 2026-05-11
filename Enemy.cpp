// Enemy.cpp
#include "Enemy.hpp"
#include <cmath>
#include <algorithm>

Enemy::Enemy(sf::Vector2f startPos, float hp, float spd, float rew, Type type)
    : Entity(startPos), baseSpeed(spd), speed(spd),
      maxHealth(hp), currentHealth(hp), reward(rew)
{
    applyTypeVisuals(type);
}

void Enemy::applyTypeVisuals(Type type) {
    float radius = 10.f;
    switch (type) {
        case Type::Basic: radius = 13.f; baseColor = sf::Color(220, 60,  60);  break;
        case Type::Fast:  radius =  9.f; baseColor = sf::Color(255, 160,  0);  break;
        case Type::Tank:  radius = 18.f; baseColor = sf::Color(100, 60,  180); break;
    }
    auto cs = std::make_unique<sf::CircleShape>(radius);
    cs->setFillColor(baseColor);
    cs->setOutlineColor(sf::Color(0, 0, 0, 180));
    cs->setOutlineThickness(1.5f);
    cs->setOrigin(sf::Vector2f(radius, radius));
    cs->setPosition(position);
    circlePtr = cs.get();
    shape     = std::move(cs);
}

void Enemy::setPath(const std::vector<sf::Vector2f>& p) {
    path = p; pathIndex = 0;
}

void Enemy::takeDamage(float dmg) {
    currentHealth -= dmg;
    if (currentHealth <= 0.f) {
        currentHealth = 0.f;
        killedByTower = true;
        kill();
    }
}

void Enemy::applySlow(float factor) {
    slowTimer = 1.5f;
    if (!isSlowed) {
        isSlowed = true;
        speed    = baseSpeed * (1.f - factor);
        circlePtr->setFillColor(sf::Color(
            static_cast<uint8_t>(baseColor.r * 0.6f),
            static_cast<uint8_t>(baseColor.g * 0.6f),
            static_cast<uint8_t>(std::min(255.f, baseColor.b * 0.6f + 120.f))
        ));
    }
}

void Enemy::update(float dt) {
    if (!alive || path.empty()) return;

    // Tick slow
    if (isSlowed) {
        slowTimer -= dt;
        if (slowTimer <= 0.f) {
            isSlowed = false;
            speed    = baseSpeed;
            circlePtr->setFillColor(baseColor);
        }
    }

    if (pathIndex >= path.size()) {
        // Signal that we've reached the end — CombatSystem handles kill + life loss
        reachedPathEnd = true;
        return;
    }

    sf::Vector2f tgt = path[pathIndex];
    sf::Vector2f dir = tgt - position;
    float dist = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    float step = speed * dt;

    if (dist <= step) { position = tgt; ++pathIndex; }
    else              { dir /= dist; position += dir * step; }

    circlePtr->setPosition(position);
}

void Enemy::render(sf::RenderWindow& win) {
    if (!alive) return;
    win.draw(*circlePtr);

    // Health bar
    const float barW  = circlePtr->getRadius() * 2.f;
    const float barH  = 4.f;
    const float barX  = position.x - circlePtr->getRadius();
    const float barY  = position.y - circlePtr->getRadius() - 8.f;
    const float ratio = std::max(0.f, currentHealth / maxHealth);

    sf::RectangleShape bg({barW, barH});
    bg.setPosition({barX, barY});
    bg.setFillColor(sf::Color(60, 60, 60, 200));
    win.draw(bg);

    if (ratio > 0.f) {
        sf::RectangleShape fg({barW * ratio, barH});
        fg.setPosition({barX, barY});
        sf::Color hpCol = (ratio > 0.5f) ? sf::Color(50,200,50)
                        : (ratio > 0.25f ? sf::Color(220,200,0) : sf::Color(220,50,50));
        fg.setFillColor(hpCol);
        win.draw(fg);
    }

    if (isSlowed) {
        sf::CircleShape ring(circlePtr->getRadius() + 3.f);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineColor(sf::Color(0, 220, 255, 180));
        ring.setOutlineThickness(2.f);
        ring.setOrigin(sf::Vector2f(ring.getRadius(), ring.getRadius()));
        ring.setPosition(position);
        win.draw(ring);
    }
}

float Enemy::getRadius() const noexcept {
    return circlePtr ? circlePtr->getRadius() : 0.f;
}
