// WaypointPath.hpp
// Holds the ordered list of pixel-space positions that enemies follow.
// The S-curve route crosses the 20×12 tile map (each tile = 64px).
//
// Pixel formula:  x = col * 64 + 32,  y = row * 64 + 32
//
// Route (col, row):
//   (0,2) → (6,2) → (6,7) → (12,7) → (12,3) → (19,3)
//   Entry left → turn down → turn right → turn up → exit right
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class WaypointPath {
public:
    // Populates the internal point list. Call once during initialisation.
    void build();

    // Returns the full ordered list of waypoints (const ref, no copy).
    const std::vector<sf::Vector2f>& getPoints() const noexcept;

    int          size()           const noexcept;
    sf::Vector2f getSpawnPoint()  const noexcept;  // first point
    sf::Vector2f getExitPoint()   const noexcept;  // last point

    // Debug visualisation — red dots at each waypoint, yellow connecting lines.
    // Call temporarily from GameEngine::render() to verify layout.
    void debugDraw(sf::RenderWindow& win) const;

private:
    std::vector<sf::Vector2f> points;
};
