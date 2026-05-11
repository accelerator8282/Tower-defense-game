// HighScore.cpp
#include "HighScore.hpp"
#include <fstream>
#include <algorithm>

const std::string HighScore::FILE_PATH = "scores.txt";

// ── load ─────────────────────────────────────────────────────────────────────
void HighScore::load() {
    scores.clear();
    std::ifstream in(FILE_PATH);
    if (!in.is_open()) return;   // first run — no file yet, silent pass

    int v;
    while (in >> v)
        scores.push_back(v);

    sortAndTrim();
}

// ── save ─────────────────────────────────────────────────────────────────────
void HighScore::save() const {
    std::ofstream out(FILE_PATH);
    if (!out.is_open()) return;  // disk full or read-only — skip silently

    for (int s : scores)
        out << s << '\n';
}

// ── submitScore ───────────────────────────────────────────────────────────────
void HighScore::submitScore(int score) {
    scores.push_back(score);
    sortAndTrim();
    save();
}

// ── sortAndTrim ───────────────────────────────────────────────────────────────
void HighScore::sortAndTrim() {
    std::sort(scores.begin(), scores.end(),
              [](int a, int b) { return a > b; });   // descending

    if (static_cast<int>(scores.size()) > MAX_SCORES)
        scores.resize(MAX_SCORES);
}

// ── isHighScore ───────────────────────────────────────────────────────────────
bool HighScore::isHighScore(int score) const {
    if (static_cast<int>(scores.size()) < MAX_SCORES) return true;
    return score > scores.back();
}

// ── getRank ───────────────────────────────────────────────────────────────────
int HighScore::getRank(int score) const {
    for (int i = 0; i < static_cast<int>(scores.size()); ++i)
        if (score >= scores[i]) return i + 1;
    return -1;
}
