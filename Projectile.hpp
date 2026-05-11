// Projectile.hpp
#pragma once
#include "Entity.hpp"
#include "TowerType.hpp"

class Projectile : public Entity {
public:
    // slowFactor: 0 = normal, 0.25 = slow tower projectile
    Projectile(sf::Vector2f pos, sf::Vector2f dir,
               float speed, int dmg, float slowFactor = 0.f,
               TowerType ttype = TowerType::Basic);

    int   getDamage()     const noexcept { return damage;     }
    float getSlowFactor() const noexcept { return slowFactor; }

    void  update(float dt)              override;
    void  render(sf::RenderWindow& win) override;
    float getRadius() const noexcept    override;

private:
    sf::Vector2f     velocity;
    int              damage;
    float            slowFactor;
    TowerType        towerType;
    sf::CircleShape* bulletPtr = nullptr;
};
