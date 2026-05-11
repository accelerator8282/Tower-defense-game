// WaveManager.cpp
#include "WaveManager.hpp"
#include "Enemy.hpp"

// ── Constructor ───────────────────────────────────────────────────────────────
WaveManager::WaveManager(const std::vector<sf::Vector2f>& p, SpawnCallback cb)
    : path(p)
    , onSpawn(std::move(cb))
{
    defineWaves();
}

// ── defineWaves ───────────────────────────────────────────────────────────────
// All 6 wave configurations. Difficulty escalates steadily:
//   Waves 1–2 introduce basics, Wave 3 surprises with speed,
//   Wave 5 tests burst damage (tanks), Wave 6 is a swarm finale.
void WaveManager::defineWaves() {
    waveDefs = {
        { 10, 1.2f, "basic" },   // Wave 1 — slow intro
        { 15, 1.0f, "basic" },   // Wave 2 — more volume
        {  8, 0.7f, "fast"  },   // Wave 3 — speed surprise
        { 12, 0.9f, "fast"  },   // Wave 4 — fast + volume
        {  6, 1.8f, "tank"  },   // Wave 5 — tanky, slow
        { 20, 0.5f, "basic" },   // Wave 6 — rush finale
    };
}

// ── startNextWave ─────────────────────────────────────────────────────────────
void WaveManager::startNextWave() {
    if (allWavesComplete()) return;

    ++currentWave;
    toSpawn    = waveDefs[currentWave].count;
    spawnTimer = 0.f;   // fire the first enemy on the very next update tick
}

// ── update ────────────────────────────────────────────────────────────────────
// Counts down the spawn timer and fires onSpawn when an enemy is due.
// WaveManager does NOT track living enemies — CombatSystem owns them.
void WaveManager::update(float dt) {
    if (toSpawn <= 0 || currentWave < 0) return;

    spawnTimer -= dt;
    if (spawnTimer <= 0.f) {
        onSpawn(makeEnemy(waveDefs[currentWave].enemyType));
        --toSpawn;
        spawnTimer = waveDefs[currentWave].spawnInterval;
    }
}

// ── isWaveActive ──────────────────────────────────────────────────────────────
// True as long as there are still enemies waiting to be spawned in this wave.
// NOTE: whether living enemies remain on-screen is tracked by CombatSystem,
// not here. GameEngine must check BOTH before starting the next wave.
bool WaveManager::isWaveActive() const noexcept {
    return toSpawn > 0;
}

// ── allWavesComplete ──────────────────────────────────────────────────────────
bool WaveManager::allWavesComplete() const noexcept {
    return currentWave >= static_cast<int>(waveDefs.size()) - 1 && toSpawn == 0;
}

// ── getWaveNumber ─────────────────────────────────────────────────────────────
int WaveManager::getWaveNumber() const noexcept {
    return currentWave + 1;   // 0-based internal → 1-based display
}

// ── makeEnemy ─────────────────────────────────────────────────────────────────
// Factory — maps a type string to Enemy constructor parameters.
// Ownership is transferred to the caller via unique_ptr.
std::unique_ptr<Enemy> WaveManager::makeEnemy(const std::string& type) const {
    sf::Vector2f startPos = path.empty() ? sf::Vector2f{} : path.front();

    std::unique_ptr<Enemy> e;

    if (type == "fast") {
        e = std::make_unique<Enemy>(startPos, 50.f, 165.f, 20.f, Enemy::Type::Fast);
    } else if (type == "tank") {
        e = std::make_unique<Enemy>(startPos, 1000.f, 60.f, 30.f, Enemy::Type::Tank);
    } else {
        // Default to "basic" for unknown types — safe fallback.
        e = std::make_unique<Enemy>(startPos, 100.f, 100.f, 20.f, Enemy::Type::Basic);
    }

    e->setPath(path);
    return e;
}
