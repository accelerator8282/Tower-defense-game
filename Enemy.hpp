// Enemy.hpp
#pragma once
#include "Entity.hpp"
#include <vector>

class Enemy : public Entity {
public:
    enum class Type { Basic, Fast, Tank };

    Enemy(sf::Vector2f startPos, float hp, float speed,
          float reward, Type type = Type::Basic);

    void setPath(const std::vector<sf::Vector2f>& p);

    // Combat
    void  takeDamage(float dmg);
    void  applySlow(float factor);   // 0.25 = 25% slower; wears off after 1.5s
    float getReward()         const noexcept { return reward;        }
    bool  wasKilledByTower()  const noexcept { return killedByTower; }
    bool  reachedEnd()        const noexcept { return reachedPathEnd; }

    void  update(float dt)              override;
    void  render(sf::RenderWindow& win) override;
    float getRadius()      const noexcept override;

private:
    float  baseSpeed;       // original speed — slow resets to this
    float  speed;           // current speed (may be reduced)
    float  maxHealth;
    float  currentHealth;
    float  reward;
    size_t pathIndex      = 0;
    bool   killedByTower  = false;
    bool   reachedPathEnd = false;

    // Slow state
    float  slowTimer      = 0.f;   // counts down; 0 = not slowed
    bool   isSlowed       = false;

    std::vector<sf::Vector2f> path;
    sf::CircleShape*          circlePtr = nullptr;

    // Base color — restored when slow wears off
    sf::Color baseColor;

    void applyTypeVisuals(Type type);
};
