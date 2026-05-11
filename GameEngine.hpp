// GameEngine.hpp
#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include "TileMap.hpp"
#include "WaveManager.hpp"
#include "CombatSystem.hpp"
#include "Economy.hpp"
#include "HighScore.hpp"

enum class Scene { MainMenu, Playing, Paused, GameOver, Victory };

struct UIButton {
    sf::RectangleShape rect;
    std::string        label;
    bool               enabled = true;

    UIButton() = default;
    UIButton(sf::Vector2f pos, sf::Vector2f size, const std::string& lbl);
    bool contains(sf::Vector2f pt) const;
};

class GameEngine {
public:
    GameEngine();
    void run();

private:
    sf::RenderWindow        window;
    std::optional<sf::Font> font;

    TileMap      tileMap;
    CombatSystem combat;
    Economy      economy;
    HighScore    highScore;
    std::unique_ptr<WaveManager> waveManager;

    Scene         scene          = Scene::MainMenu;
    bool          waveInProgress = false;

    // Layout constants
    // Window: 1280 x 768
    // Top HUD bar: 32px
    // Map area: 1280 x 672 (below HUD, above bottom bar)
    // Bottom bar: 96px (tower buttons + next wave + info)
    static constexpr int WIN_W       = 1280;
    static constexpr int WIN_H       = 768;
    static constexpr int HUD_H       = 32;
    static constexpr int BOTTOM_H    = 128;
    static constexpr int MAP_H       = WIN_H - HUD_H - BOTTOM_H; // 608
    // Full map width (no side sidebar)
    static constexpr int MAP_W       = WIN_W; // 1280

    TowerType     selectedType    = TowerType::Basic;
    Tower*        hoveredTower    = nullptr;
    Tower*        selectedTower   = nullptr;

    // Bottom bar buttons (tower purchase)
    UIButton      btnBasic, btnSniper, btnSlow;
    UIButton      btnNextWave;

    // Upgrade popup (shown near hovered tower)
    UIButton      btnUpgrade;
    bool          showUpgradePopup = false;
    sf::Vector2f  upgradePopupPos;

    // Main menu / pause / game-over buttons
    UIButton      btnStart, btnQuit, btnResume, btnMenu;

    std::string   notifText;
    float         notifTimer   = 0.f;
    float         victoryTimer = 0.f;   // counts up after victory; buttons appear after delay

    void initUI();
    void processEvents();
    void update(float dt);
    void render();

    void renderMainMenu();
    void renderGame();
    void renderGameOver();
    void renderVictory();
    void renderPaused();

    void drawHUD();
    void drawBottomBar();
    void drawTowerHoverPopup();

    void handlePlayClick(sf::Vector2i mousePos);
    void handleRightClick(sf::Vector2i mousePos);
    void showNotif(const std::string& msg);

    void drawText(const std::string& str, float x, float y,
                  unsigned size, sf::Color col = sf::Color::White);
    void drawCentred(const std::string& str, float cx, float y,
                     unsigned size, sf::Color col = sf::Color::White);
    void drawButton(const UIButton& btn, sf::Color fill,
                    sf::Color textCol = sf::Color::White);

    void startNewGame();
    void applyLetterbox();
    sf::Vector2f mapMouse(sf::Vector2i physicalPos) const;
};
