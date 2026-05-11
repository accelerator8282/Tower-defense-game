// WaypointPath.cpp — updated for 20-col x 9-row map, 32px HUD offset
#include "WaypointPath.hpp"

void WaypointPath::build() {
    // Tile pixel centre: x = col*64+32, y = row*64+32 + 32(HUD)
    // Path: entry row1 left, down col4, across row6, up col14, exit row2 right
    points = {
        {  32.f, 1*64 + 32 + 32 },   // col0,  row1 — spawn
        { 4*64+32, 1*64 + 32 + 32 },  // col4,  row1 — turn down
        { 4*64+32, 6*64 + 32 + 32 },  // col4,  row6 — turn right
        { 14*64+32, 6*64 + 32 + 32 }, // col14, row6 — turn up
        { 14*64+32, 2*64 + 32 + 32 }, // col14, row2 — turn right
        { 1280.f,   2*64 + 32 + 32 }, // col19+ — exit
    };
}

const std::vector<sf::Vector2f>& WaypointPath::getPoints() const noexcept {
    return points;
}
int WaypointPath::size() const noexcept { return (int)points.size(); }
sf::Vector2f WaypointPath::getSpawnPoint() const noexcept {
    return points.empty() ? sf::Vector2f{} : points.front();
}
sf::Vector2f WaypointPath::getExitPoint() const noexcept {
    return points.empty() ? sf::Vector2f{} : points.back();
}
void WaypointPath::debugDraw(sf::RenderWindow& win) const {
    for (int i = 0; i+1 < (int)points.size(); ++i) {
        sf::Vertex line[2];
        line[0].position = points[i];   line[0].color = sf::Color::Yellow;
        line[1].position = points[i+1]; line[1].color = sf::Color::Yellow;
        win.draw(line, 2, sf::PrimitiveType::Lines);
    }
}
