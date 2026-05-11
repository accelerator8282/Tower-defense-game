// Projectile.cpp
#include "Projectile.hpp"

Projectile::Projectile(sf::Vector2f pos, sf::Vector2f dir,
                       float speed, int dmg, float sf, TowerType tt)
    : Entity(pos), velocity(dir * speed), damage(dmg),
      slowFactor(sf), towerType(tt)
{
    float r = 4.f;
    sf::Color fill, outline;
    switch (towerType) {
        case TowerType::Basic:
            fill    = sf::Color(255, 230, 50);
            outline = sf::Color(255, 140, 0);
            break;
        case TowerType::Sniper:
            r       = 5.f;
            fill    = sf::Color(255, 80, 80);
            outline = sf::Color(180, 20, 20);
            break;
        case TowerType::Slow:
            fill    = sf::Color(80, 220, 255);
            outline = sf::Color(0,  140, 200);
            break;
    }

    auto cs = std::make_unique<sf::CircleShape>(r);
    cs->setFillColor(fill);
    cs->setOutlineColor(outline);
    cs->setOutlineThickness(1.f);
    cs->setOrigin(sf::Vector2f(r, r));
    cs->setPosition(position);
    bulletPtr = cs.get();
    shape     = std::move(cs);
}

void Projectile::update(float dt) {
    if (!alive) return;
    position += velocity * dt;
    bulletPtr->setPosition(position);
    constexpr float M = 100.f;
    if (position.x < -M || position.x > 1280.f+M ||
        position.y < -M || position.y > 768.f +M)
        kill();
}

void Projectile::render(sf::RenderWindow& win) {
    if (alive) win.draw(*bulletPtr);
}

float Projectile::getRadius() const noexcept {
    return bulletPtr ? bulletPtr->getRadius() : 0.f;
}
