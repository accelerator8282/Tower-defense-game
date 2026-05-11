// Economy.cpp
#include "Economy.hpp"
#include <algorithm>    // std::max

Economy::Economy(int startMoney, int startLives)
    : money(startMoney), lives(startLives) {}

// ── spend ────────────────────────────────────────────────────────────────────
bool Economy::spend(int cost) {
    if (money < cost) return false;
    money -= cost;
    return true;
}

// ── earn ─────────────────────────────────────────────────────────────────────
// Reward is a float (matches Enemy::getReward()) but stored as int.
void Economy::earn(float amount) {
    int coins = static_cast<int>(amount);
    money += coins;
    score += coins;   // every coin also adds to score
}

// ── loseLife ─────────────────────────────────────────────────────────────────
void Economy::loseLife(int n) {
    lives = std::max(0, lives - n);
}

// ── recordKill ───────────────────────────────────────────────────────────────
void Economy::recordKill() {
    ++kills;
    score += 10;
}

// ── addWaveBonus ─────────────────────────────────────────────────────────────
// Scales with wave number so later waves stay lucrative.
void Economy::addWaveBonus(int waveNumber) {
    score += waveNumber * 500;
    money += waveNumber * 25;
}

// ── isGameOver ───────────────────────────────────────────────────────────────
bool Economy::isGameOver() const noexcept {
    return lives <= 0;
}
