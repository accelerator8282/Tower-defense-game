// HighScore.hpp
// Persists the top 5 scores to "scores.txt" using std::fstream.
// No SFML dependency — pure C++ standard library.
//
// Lifecycle:
//   1. GameEngine calls load()         on startup.
//   2. GameEngine calls submitScore()  on game over.
//   3. GameOver screen reads getTopScores() for display.
#pragma once
#include <vector>
#include <string>

class HighScore {
public:
    static constexpr int MAX_SCORES = 5;

    // Silent on missing file (first run) — scores vector stays empty.
    void load();

    // Insert, sort descending, trim to MAX_SCORES, then persist.
    void submitScore(int score);

    // Manual flush (submitScore already calls this internally).
    void save() const;

    // Read-only access for the GameOver / HUD display.
    const std::vector<int>& getTopScores() const noexcept { return scores; }

    // True if `score` would appear in the top-MAX_SCORES table.
    bool isHighScore(int score) const;

    // Returns 1-based rank, or -1 if not in top MAX_SCORES.
    int getRank(int score) const;

private:
    std::vector<int> scores;
    static const std::string FILE_PATH;   // "scores.txt"

    void sortAndTrim();
};
