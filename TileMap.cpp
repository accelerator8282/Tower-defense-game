// TileMap.cpp — 20 cols x 9 rows, 64px tiles → 1280x576 map area (with 32px HUD top)
#include "TileMap.hpp"

sf::Color TileMap::tileColor(TileType t) noexcept {
    switch (t) {
        case TileType::Grass: return sf::Color(55, 110, 45);
        case TileType::Path:  return sf::Color(175, 150, 85);
        default:              return sf::Color(80, 80, 80);
    }
}

void TileMap::init() { buildGrid(); buildWaypoints(); }

void TileMap::buildGrid() {
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            Tile& t    = grid[r][c];
            t.type     = TileType::Grass;
            t.hasTower = false;
            t.rect.setSize({TILE_SIZE - 1.f, TILE_SIZE - 1.f});
            // y offset = HUD_H (32px)
            t.rect.setPosition({(float)(c * TILE_SIZE), (float)(r * TILE_SIZE + HUD_H)});
            t.rect.setFillColor(tileColor(TileType::Grass));
            t.rect.setOutlineColor(sf::Color(35, 80, 25));
            t.rect.setOutlineThickness(0.5f);
        }
    }

    auto markPath = [&](int row, int col) {
        if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return;
        Tile& t = grid[row][col];
        t.type  = TileType::Path;
        t.rect.setFillColor(tileColor(TileType::Path));
        t.rect.setOutlineColor(sf::Color(135, 115, 55));
    };

    // S-curve path across the 20x9 grid
    // Entry: row 1, cols 0-4
    for (int c = 0; c <= 4; ++c)  markPath(1, c);
    // Descent: col 4, rows 1-6
    for (int r = 1; r <= 6; ++r)  markPath(r, 4);
    // Mid: row 6, cols 4-14
    for (int c = 4; c <= 14; ++c) markPath(6, c);
    // Ascent: col 14, rows 2-6
    for (int r = 2; r <= 6; ++r)  markPath(r, 14);
    // Exit: row 2, cols 14-19
    for (int c = 14; c <= 19; ++c) markPath(2, c);
}

void TileMap::buildWaypoints() { waypointPath.build(); }

void TileMap::render(sf::RenderWindow& win) const {
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            win.draw(grid[r][c].rect);
}

bool TileMap::canPlaceTower(int col, int row) const noexcept {
    if (col < 0 || col >= COLS || row < 0 || row >= ROWS) return false;
    const Tile& t = grid[row][col];
    return (t.type == TileType::Grass) && !t.hasTower;
}

void TileMap::placeTower(int col, int row) {
    if (canPlaceTower(col, row)) grid[row][col].hasTower = true;
}

void TileMap::removeTower(int col, int row) {
    if (col >= 0 && col < COLS && row >= 0 && row < ROWS)
        grid[row][col].hasTower = false;
}

sf::Vector2i TileMap::pixelToGrid(sf::Vector2f pixel) noexcept {
    // Subtract HUD_H from y before dividing
    return {
        static_cast<int>(pixel.x) / TILE_SIZE,
        static_cast<int>(pixel.y - HUD_H) / TILE_SIZE
    };
}

const WaypointPath& TileMap::getWaypointPath() const noexcept {
    return waypointPath;
}
