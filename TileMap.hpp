// TileMap.hpp — 20 cols x 9 rows, 64px tiles → 1280x576 map area
#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include "WaypointPath.hpp"

enum class TileType { Grass, Path };

struct Tile {
    TileType           type     = TileType::Grass;
    sf::RectangleShape rect;
    bool               hasTower = false;
};

class TileMap {
public:
    static constexpr int TILE_SIZE = 64;
    static constexpr int COLS      = 20;
    static constexpr int ROWS      = 9;
    static constexpr int HUD_H     = 32;  // top HUD offset

    void init();
    void render(sf::RenderWindow& win) const;

    bool canPlaceTower(int col, int row) const noexcept;
    void placeTower(int col, int row);
    void removeTower(int col, int row);

    static sf::Vector2i pixelToGrid(sf::Vector2f pixel) noexcept;
    const WaypointPath& getWaypointPath() const noexcept;

private:
    std::array<std::array<Tile, COLS>, ROWS> grid;
    WaypointPath waypointPath;

    void buildGrid();
    void buildWaypoints();
    static sf::Color tileColor(TileType t) noexcept;
};
