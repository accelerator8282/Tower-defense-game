// Economy.hpp
// Tracks player money, lives, score, and kill count.
// No SFML dependency — pure C++ logic. Designed to be read-only from
// outside except via the explicit mutating methods below.
#pragma once

class Economy {
public:
    Economy(int startMoney = 150, int startLives = 20);

    // ── Mutating API ──────────────────────────────────────────
    // Returns false (and does NOT deduct) if cost exceeds current money.
    bool spend(int cost);

    // Tower-kill reward. Also increments score by the amount earned.
    void earn(float amount);

    // Called when an enemy exits the path. Clamped to 0.
    void loseLife(int n = 1);

    // Called when a tower kill is confirmed — adds +10 score per kill.
    void recordKill();

    // Called when a wave is fully cleared by GameEngine.
    // Bonus scales with wave number to keep later waves rewarding.
    void addWaveBonus(int waveNumber);

    // ── Queries ───────────────────────────────────────────────
    bool isGameOver()    const noexcept;
    int  getMoney()      const noexcept { return money; }
    int  getLives()      const noexcept { return lives; }
    int  getScore()      const noexcept { return score; }
    int  getKillCount()  const noexcept { return kills; }

private:
    int money;
    int lives;
    int score = 0;
    int kills = 0;
};
