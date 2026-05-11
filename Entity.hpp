// Entity.hpp
// Abstract base class for all game objects.
// Uses a std::unique_ptr for the SFML shape so ownership is unambiguous
// and the destructor is trivial — no manual delete needed in any subclass.
#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

class Entity {
protected:
    sf::Vector2f position;
    bool         alive  = true;

    // Subclasses assign their concrete shape here once in the constructor.
    // Entity owns it exclusively; subclasses keep a typed raw-observer pointer
    // (e.g. `sf::CircleShape* circle = ...`) for fast access without casting.
    std::unique_ptr<sf::Shape> shape;

public:
    explicit Entity(sf::Vector2f pos)
        : position(pos), alive(true) {}

    // Non-copyable — shapes and game-state don't make sense to copy.
    Entity(const Entity&)            = delete;
    Entity& operator=(const Entity&) = delete;

    virtual ~Entity() = default;

    // ── Core interface ────────────────────────────────────────
    virtual void update(float dt)              = 0;
    virtual void render(sf::RenderWindow& win) = 0;

    // ── Accessors ─────────────────────────────────────────────
    sf::Vector2f getPosition() const noexcept { return position; }
    bool         isAlive()     const noexcept { return alive;    }
    void         kill()              noexcept { alive = false;   }

    // Collision radius — subclasses override for circle-based hit detection.
    virtual float getRadius() const noexcept { return 0.f; }
};
