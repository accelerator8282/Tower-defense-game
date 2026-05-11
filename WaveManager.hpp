// WaveManager.hpp
// Controls wave progression and enemy spawning timing.
//
// Ownership model (fixed from original):
//   WaveManager is a PURE SPAWNER — it creates enemies and immediately
//   transfers ownership (via unique_ptr) to CombatSystem. It does NOT
//   maintain an `enemies` vector. This eliminates the double-ownership
//   bug present in the original code where both WaveManager and
//   CombatSystem held pointers to the same Enemy objects.
//
// Usage:
//   1. Construct with the waypoint path from TileMap.
//   2. Call startNextWave() to begin wave 1.
//   3. Call update(dt) every frame; it invokes onSpawn for each new enemy.
//   4. When !isWaveActive() && !allWavesComplete(), call startNextWave() again.
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include <memory>

class Enemy;  // forward — WaveManager.hpp does not need full definition

struct WaveDef {
    int         count;          // number of enemies in this wave
    float       spawnInterval;  // seconds between individual spawns
    std::string enemyType;      // "basic" | "fast" | "tank"
};

class WaveManager {
public:
    // `path` — waypoint positions forwarded to each spawned enemy.
    // `onSpawn` — callback invoked for each new enemy; CombatSystem
    //             uses this to take ownership of the unique_ptr.
    using SpawnCallback = std::function<void(std::unique_ptr<Enemy>)>;

    explicit WaveManager(const std::vector<sf::Vector2f>& path,
                         SpawnCallback                    onSpawn);

    // Advance to the next wave. Safe to call before wave 1 starts.
    void startNextWave();

    // Tick the spawn timer; fires onSpawn when an enemy is due.
    // Does NOT manage enemy lifetimes — that's CombatSystem's job.
    void update(float dt);

    // State queries — GameEngine polls each frame.
    bool isWaveActive()     const noexcept;  // enemies still pending
    bool allWavesComplete() const noexcept;  // all waves spawned
    int  getWaveNumber()    const noexcept;  // 1-based, for HUD

private:
    const std::vector<sf::Vector2f>& path;
    SpawnCallback                    onSpawn;

    std::vector<WaveDef> waveDefs;
    int   currentWave = -1;
    int   toSpawn     = 0;
    float spawnTimer  = 0.f;

    void defineWaves();

    // Factory — creates an Enemy with stats matching the type string.
    std::unique_ptr<Enemy> makeEnemy(const std::string& type) const;
};
