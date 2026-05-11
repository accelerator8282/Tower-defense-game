// GameEngine.cpp
#include "GameEngine.hpp"
#include <string>
#include <cmath>

UIButton::UIButton(sf::Vector2f pos, sf::Vector2f size, const std::string& lbl)
    : label(lbl)
{
    rect.setPosition(pos);
    rect.setSize(size);
    rect.setOutlineThickness(2.f);
    rect.setOutlineColor(sf::Color(80, 80, 80));
}

bool UIButton::contains(sf::Vector2f pt) const {
    return rect.getGlobalBounds().contains(pt);
}

// ─── Constructor ──────────────────────────────────────────────────────────────

GameEngine::GameEngine()
    : window(sf::VideoMode({(unsigned)WIN_W, (unsigned)WIN_H}), "Tower Defense")
    , economy(150, 20)
{
    window.setFramerateLimit(60);
    applyLetterbox();
    sf::Font f;
    if (f.openFromFile("arial.ttf")) font = std::move(f);
    tileMap.init();
    highScore.load();
    initUI();
}

void GameEngine::initUI() {
    // Bottom bar: y starts at HUD_H + MAP_H = 32 + 608 = 640
    const float BY  = (float)(HUD_H + MAP_H);   // 640
    const float BTW = 160.f;
    const float BTH = 50.f;
    const float BPAD = 16.f;

    // Tower buy buttons — left side of bottom bar
    btnBasic  = UIButton({BPAD,                  BY + 12.f}, {BTW, BTH}, "Basic  $50");
    btnSniper = UIButton({BPAD + (BTW+8.f),      BY + 12.f}, {BTW, BTH}, "Sniper $75");
    btnSlow   = UIButton({BPAD + 2*(BTW+8.f),    BY + 12.f}, {BTW, BTH}, "Slow   $60");

    // Next wave button — right side of bottom bar
    btnNextWave = UIButton({(float)WIN_W - BTW - BPAD, BY + 12.f}, {BTW, BTH}, "Next Wave");

    // Upgrade button — positioned dynamically near hovered tower
    btnUpgrade = UIButton({0.f, 0.f}, {140.f, 44.f}, "Upgrade");

    // Main menu buttons (centred)
    btnStart = UIButton({440.f, 340.f}, {400.f, 60.f}, "Start Game");
    btnQuit  = UIButton({440.f, 430.f}, {400.f, 60.f}, "Quit");

    // Pause overlay buttons
    btnResume = UIButton({440.f, 340.f}, {400.f, 60.f}, "Resume");
    btnMenu   = UIButton({440.f, 430.f}, {400.f, 60.f}, "Main Menu");
}

// ─── startNewGame ─────────────────────────────────────────────────────────────

void GameEngine::startNewGame() {
    economy = Economy(150, 20);
    tileMap.init();
    combat  = CombatSystem();

    combat.onEnemyKilled  = [this](float r){ economy.earn(r); economy.recordKill(); };
    combat.onEnemyEscaped = [this](){ economy.loseLife(); };

    waveManager = std::make_unique<WaveManager>(
        tileMap.getWaypointPath().getPoints(),
        [this](std::unique_ptr<Enemy> e){ combat.spawnEnemy(std::move(e)); }
    );

    waveManager->startNextWave();
    waveInProgress    = true;
    selectedTower     = nullptr;
    hoveredTower      = nullptr;
    showUpgradePopup  = false;
    selectedType      = TowerType::Basic;
    victoryTimer      = 0.f;
    scene             = Scene::Playing;
}

// ─── applyLetterbox ───────────────────────────────────────────────────────────
// Keeps the game at 1280x768 internally. Black bars fill any leftover space.

void GameEngine::applyLetterbox() {
    sf::Vector2u win = window.getSize();
    float winRatio  = (float)win.x / (float)win.y;
    float gameRatio = (float)WIN_W  / (float)WIN_H;

    float scaleX = 1.f, scaleY = 1.f;
    float offsetX = 0.f, offsetY = 0.f;

    if (winRatio > gameRatio) {
        // Wider than game → pillarbox
        scaleX  = gameRatio / winRatio;
        offsetX = (1.f - scaleX) / 2.f;
    } else {
        // Taller than game → letterbox
        scaleY  = winRatio / gameRatio;
        offsetY = (1.f - scaleY) / 2.f;
    }

    sf::View view(sf::FloatRect({0.f, 0.f}, {(float)WIN_W, (float)WIN_H}));
    view.setViewport(sf::FloatRect({offsetX, offsetY}, {scaleX, scaleY}));
    window.setView(view);
}

// Maps a physical window mouse position → virtual 1280x768 game coordinates.
sf::Vector2f GameEngine::mapMouse(sf::Vector2i physPos) const {
    return window.mapPixelToCoords(physPos);
}

// ─── run ──────────────────────────────────────────────────────────────────────

void GameEngine::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;
        processEvents();
        update(dt);
        render();
    }
}

// ─── processEvents ────────────────────────────────────────────────────────────

void GameEngine::processEvents() {
    while (const auto ev = window.pollEvent()) {
        if (ev->is<sf::Event::Closed>()) { window.close(); return; }

        if (ev->is<sf::Event::Resized>()) {
            applyLetterbox();
        }

        if (const auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
            // Map physical pixel → virtual game coords
            sf::Vector2f mf = mapMouse(mb->position);
            sf::Vector2i mi((int)mf.x, (int)mf.y);

            if (mb->button == sf::Mouse::Button::Left) {
                // ── Main Menu ──
                if (scene == Scene::MainMenu) {
                    if (btnStart.contains(mf)) startNewGame();
                    if (btnQuit.contains(mf))  window.close();
                }
                // ── Paused ──
                else if (scene == Scene::Paused) {
                    if (btnResume.contains(mf)) scene = Scene::Playing;
                    if (btnMenu.contains(mf))   scene = Scene::MainMenu;
                }
                // ── Playing ──
                else if (scene == Scene::Playing) {
                    // Bottom bar tower buy buttons
                    if (btnBasic.contains(mf))  { selectedType = TowerType::Basic;  selectedTower = nullptr; showUpgradePopup = false; }
                    if (btnSniper.contains(mf)) { selectedType = TowerType::Sniper; selectedTower = nullptr; showUpgradePopup = false; }
                    if (btnSlow.contains(mf))   { selectedType = TowerType::Slow;   selectedTower = nullptr; showUpgradePopup = false; }

                    if (btnNextWave.enabled && btnNextWave.contains(mf)) {
                        if (!waveManager->isWaveActive() && !waveManager->allWavesComplete()) {
                            economy.addWaveBonus(waveManager->getWaveNumber());
                            waveManager->startNextWave();
                            waveInProgress = true;
                        }
                    }

                    // Upgrade popup button click
                    if (showUpgradePopup && btnUpgrade.enabled && btnUpgrade.contains(mf) && hoveredTower) {
                        if (hoveredTower->getLevel() >= Tower::MAX_LEVEL) {
                            showNotif("Already max level!");
                        } else {
                            int cost = Tower::upgradeCost(hoveredTower->getLevel());
                            if (!economy.spend(cost)) {
                                showNotif("Not enough money! ($" + std::to_string(cost) + ")");
                            } else {
                                hoveredTower->upgrade();
                            }
                        }
                    }

                    // Map click = place or select tower
                    if (mi.y >= HUD_H && mi.y < HUD_H + MAP_H)
                        handlePlayClick(mi);
                }
                // ── Game Over / Victory ──
                else if (scene == Scene::GameOver || scene == Scene::Victory) {
                    bool buttonsActive = (scene == Scene::GameOver) || (victoryTimer >= 2.f);
                    if (buttonsActive) {
                        if (btnStart.contains(mf)) startNewGame();
                        if (btnQuit.contains(mf))  window.close();
                    }
                }
            }

            if (mb->button == sf::Mouse::Button::Right && scene == Scene::Playing) {
                if (mi.y >= HUD_H && mi.y < HUD_H + MAP_H)
                    handleRightClick(mi);
            }
        }

        if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::Escape) {
                if (scene == Scene::Playing) scene = Scene::Paused;
                else if (scene == Scene::Paused) scene = Scene::Playing;
            }
            // F11 — toggle fullscreen
            if (kp->code == sf::Keyboard::Key::F11) {
                static bool isFullscreen = false;
                isFullscreen = !isFullscreen;
                if (isFullscreen) {
                    window.create(sf::VideoMode::getDesktopMode(), "Tower Defense",
                                  sf::State::Fullscreen);
                } else {
                    window.create(sf::VideoMode({(unsigned)WIN_W, (unsigned)WIN_H}),
                                  "Tower Defense");
                }
                window.setFramerateLimit(60);
                applyLetterbox();
            }
            if (scene == Scene::Playing) {
                if (kp->code == sf::Keyboard::Key::Num1) selectedType = TowerType::Basic;
                if (kp->code == sf::Keyboard::Key::Num2) selectedType = TowerType::Sniper;
                if (kp->code == sf::Keyboard::Key::Num3) selectedType = TowerType::Slow;
            }
        }
    }

    // Track hovered tower — map physical mouse pos → virtual coords
    if (scene == Scene::Playing) {
        sf::Vector2i physMp = sf::Mouse::getPosition(window);
        sf::Vector2f mp     = mapMouse(physMp);

        // Reset range on all towers every frame
        combat.resetAllRangeDisplay();

        hoveredTower     = nullptr;
        showUpgradePopup = false;

        if (mp.y >= HUD_H && mp.y < HUD_H + MAP_H) {
            Tower* t = combat.getTowerAt(mp, 36.f);
            if (t) {
                hoveredTower = t;
                hoveredTower->setShowRange(true);
                showUpgradePopup = true;

                // Popup is 150px wide, 110px tall — center it above the tower
                sf::Vector2f tpos = hoveredTower->getPosition();
                const float popW = 150.f, popH = 110.f;
                float popX = tpos.x - popW / 2.f;   // horizontally centered on tower
                float popY = tpos.y - popH - 12.f;  // just above the tower body

                // Clamp inside the map area
                if (popX < 4.f) popX = 4.f;
                if (popX + popW > WIN_W - 4.f) popX = WIN_W - popW - 4.f;
                if (popY < (float)HUD_H + 4.f) popY = tpos.y + 20.f + 4.f; // flip below if no room
                upgradePopupPos = {popX, popY};
                btnUpgrade.rect.setPosition(upgradePopupPos + sf::Vector2f(5.f, 68.f));
            }
        }
    }
}

// ─── handlePlayClick ──────────────────────────────────────────────────────────

void GameEngine::handlePlayClick(sf::Vector2i mousePos) {
    sf::Vector2f mpf((float)mousePos.x, (float)mousePos.y);

    // If clicking an existing tower, just select it (don't place)
    Tower* t = combat.getTowerAt(mpf, 32.f);
    if (t) { selectedTower = t; return; }

    sf::Vector2i grid = TileMap::pixelToGrid(mpf);

    if (!tileMap.canPlaceTower(grid.x, grid.y)) return;

    int cost = Tower::buyCost(selectedType);
    if (!economy.spend(cost)) {
        showNotif("Not enough money! ($" + std::to_string(cost) + ")");
        return;
    }

    tileMap.placeTower(grid.x, grid.y);

    // Center tower in the tile — must account for HUD offset
    sf::Vector2f tPos(
        (float)(grid.x * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2),
        (float)(grid.y * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2) + (float)HUD_H
    );

    combat.placeTower(std::make_unique<Tower>(tPos, selectedType));
    selectedTower = nullptr;
}

void GameEngine::handleRightClick(sf::Vector2i mousePos) {
    Tower* t = combat.getTowerAt(sf::Vector2f((float)mousePos.x, (float)mousePos.y), 32.f);
    selectedTower = t;
}

void GameEngine::showNotif(const std::string& msg) {
    notifText  = msg;
    notifTimer = 2.f;
}

// ─── update ───────────────────────────────────────────────────────────────────

void GameEngine::update(float dt) {
    if (notifTimer > 0.f) notifTimer -= dt;
    if (scene == Scene::Victory) { victoryTimer += dt; return; }
    if (scene != Scene::Playing) return;

    waveManager->update(dt);
    combat.update(dt);

    bool spawnsDone  = !waveManager->isWaveActive();
    bool screenClear = !combat.hasLivingEnemies();

    if (waveInProgress && spawnsDone && screenClear) {
        waveInProgress = false;
        if (waveManager->allWavesComplete()) {
            highScore.submitScore(economy.getScore());
            victoryTimer = 0.f;
            scene = Scene::Victory;
            return;
        }
    }

    if (economy.isGameOver()) {
        highScore.submitScore(economy.getScore());
        scene = Scene::GameOver;
    }

    btnNextWave.enabled = !waveManager->isWaveActive()
                        && !combat.hasLivingEnemies()
                        && !waveManager->allWavesComplete();

    // Upgrade button availability
    if (hoveredTower && hoveredTower->getLevel() < Tower::MAX_LEVEL) {
        int cost = Tower::upgradeCost(hoveredTower->getLevel());
        btnUpgrade.label   = "Upgrade $" + std::to_string(cost);
        btnUpgrade.enabled = (economy.getMoney() >= cost);
    } else if (hoveredTower) {
        btnUpgrade.label   = "MAX LEVEL";
        btnUpgrade.enabled = false;
    }
}

// ─── render ───────────────────────────────────────────────────────────────────

void GameEngine::render() {
    applyLetterbox();
    window.clear(sf::Color(0, 0, 0));   // black fills letterbox/pillarbox bars
    switch (scene) {
        case Scene::MainMenu: renderMainMenu(); break;
        case Scene::Playing:  renderGame();     break;
        case Scene::Paused:   renderGame(); renderPaused(); break;
        case Scene::GameOver: renderGame(); renderGameOver(); break;
        case Scene::Victory:  renderGame(); renderVictory();  break;
    }
    window.display();
}

// ─── renderMainMenu ───────────────────────────────────────────────────────────

void GameEngine::renderMainMenu() {
    window.clear(sf::Color(12, 16, 30));

    sf::RectangleShape card({700.f, 120.f});
    card.setFillColor(sf::Color(20, 30, 60, 220));
    card.setOutlineColor(sf::Color(60, 100, 200));
    card.setOutlineThickness(2.f);
    card.setPosition({290.f, 160.f});
    window.draw(card);

    drawCentred("TOWER DEFENSE", 640.f, 180.f, 52u, sf::Color(100, 180, 255));
    drawCentred("Protect the base. Build smart.", 640.f, 246.f, 20u, sf::Color(160, 160, 180));

    drawButton(btnStart, sf::Color(30, 80, 160));
    drawButton(btnQuit,  sf::Color(80, 30, 30));

    drawCentred("1 Basic  |  2 Sniper  |  3 Slow", 640.f, 530.f, 18u, sf::Color(120,120,140));

    drawCentred("HIGH SCORES", 640.f, 570.f, 22u, sf::Color(255, 200, 0));
    const auto& top = highScore.getTopScores();
    for (int i = 0; i < (int)top.size(); ++i)
        drawCentred("#" + std::to_string(i+1) + "  " + std::to_string(top[i]),
                    640.f, 600.f + i*26.f, 18u);
}

// ─── renderGame ───────────────────────────────────────────────────────────────

void GameEngine::renderGame() {
    tileMap.render(window);
    combat.render(window);
    drawHUD();
    drawBottomBar();
    drawTowerHoverPopup();

    if (notifTimer > 0.f && font) {
        float alpha = std::min(255.f, notifTimer * 200.f);
        sf::Color nc(255, 100, 100, (uint8_t)alpha);
        drawCentred(notifText, (float)(WIN_W / 2), (float)(HUD_H + MAP_H / 2), 20u, nc);
    }
}

// ─── drawHUD ──────────────────────────────────────────────────────────────────

void GameEngine::drawHUD() {
    sf::RectangleShape bar({(float)WIN_W, (float)HUD_H});
    bar.setPosition({0.f, 0.f});
    bar.setFillColor(sf::Color(10, 10, 25, 240));
    window.draw(bar);
    if (!font) return;

    int wave = waveManager ? waveManager->getWaveNumber() : 0;
    drawText("$" + std::to_string(economy.getMoney()),        8.f,   6.f, 18u, sf::Color(80,255,80));
    drawText("Hearts: " + std::to_string(economy.getLives()), 180.f, 6.f, 18u, sf::Color(255,80,80));
    drawText("Score: " + std::to_string(economy.getScore()),  380.f, 6.f, 18u, sf::Color(255,220,50));
    drawText("Wave: " + std::to_string(wave),                 580.f, 6.f, 18u, sf::Color(150,200,255));
    drawText("Kills: " + std::to_string(economy.getKillCount()), 720.f, 6.f, 18u, sf::Color(200,200,200));
    drawText("[1]Basic [2]Sniper [3]Slow  RClick=select ESC=pause",
             900.f, 8.f, 12u, sf::Color(100,100,120));
}

// ─── drawBottomBar ────────────────────────────────────────────────────────────

void GameEngine::drawBottomBar() {
    const float BY = (float)(HUD_H + MAP_H);

    // Panel background
    sf::RectangleShape panel({(float)WIN_W, (float)BOTTOM_H});
    panel.setPosition({0.f, BY});
    panel.setFillColor(sf::Color(14, 18, 36));
    panel.setOutlineColor(sf::Color(40, 60, 120));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    if (!font) return;

    // Section label
    drawText("TOWERS", 16.f, BY + 68.f, 13u, sf::Color(140,160,220));

    // Tower buy buttons with state
    auto drawTowerBtn = [&](UIButton& btn, TowerType t, sf::Color base) {
        int cost = Tower::buyCost(t);
        bool canAfford = economy.getMoney() >= cost;
        bool selected  = (selectedType == t && !selectedTower);
        sf::Color fill;
        if (!canAfford)  fill = sf::Color(50, 50, 50, 200);
        else if (selected) fill = base;
        else             fill = sf::Color(base.r/2, base.g/2, base.b/2, 200);
        sf::Color tc = canAfford ? sf::Color::White : sf::Color(120,120,120);
        drawButton(btn, fill, tc);
    };

    drawTowerBtn(btnBasic,  TowerType::Basic,  sf::Color(60,  100, 220));
    drawTowerBtn(btnSniper, TowerType::Sniper, sf::Color(200,  50,  50));
    drawTowerBtn(btnSlow,   TowerType::Slow,   sf::Color(30,  180, 180));

    // Small subtitles under each button
    auto pos = [](const UIButton& b){ return b.rect.getPosition(); };
    drawText("Balanced, fast",       pos(btnBasic).x,  BY + 68.f, 11u, sf::Color(130,130,160));
    drawText("High DMG, slow fire",  pos(btnSniper).x, BY + 68.f, 11u, sf::Color(130,130,160));
    drawText("-25% spd on hit",      pos(btnSlow).x,   BY + 68.f, 11u, sf::Color(130,130,160));

    // Enemy legend — center
    float lx = 560.f;
    drawText("ENEMIES:", lx, BY + 8.f,  13u, sf::Color(140,140,160));
    drawText("Red=Basic(100HP)",  lx,       BY + 28.f, 11u, sf::Color(220,80,80));
    drawText("Ora=Fast(60HP)",    lx,       BY + 44.f, 11u, sf::Color(255,160,0));
    drawText("Pur=Tank(1000HP)",  lx,       BY + 60.f, 11u, sf::Color(140,80,220));
    drawText("Cyan ring=Slowed",  lx,       BY + 76.f, 11u, sf::Color(0,200,220));

    // Next wave button — far right
    if (btnNextWave.enabled) {
        drawButton(btnNextWave, sf::Color(30, 120, 40));
        drawText("Wave clear bonus!", btnNextWave.rect.getPosition().x,
                 BY + 70.f, 11u, sf::Color(80,200,80));
    } else {
        sf::Color grey(40,40,50,180);
        btnNextWave.rect.setFillColor(grey);
        window.draw(btnNextWave.rect);
        sf::Text t(*font, "Next Wave", 16u);
        t.setFillColor(sf::Color(60,60,70));
        auto bp  = btnNextWave.rect.getPosition();
        auto bsz = btnNextWave.rect.getSize();
        auto lb  = t.getLocalBounds();
        t.setPosition({bp.x + (bsz.x - lb.size.x)/2.f, bp.y + (bsz.y - lb.size.y)/2.f - 4.f});
        window.draw(t);
    }
}

// ─── drawTowerHoverPopup ──────────────────────────────────────────────────────

void GameEngine::drawTowerHoverPopup() {
    if (!showUpgradePopup || !hoveredTower || !font) return;

    int lv = hoveredTower->getLevel();
    std::string tname = Tower::typeName(hoveredTower->getType());

    // Popup card
    float pw = 150.f, ph = lv < Tower::MAX_LEVEL ? 110.f : 90.f;
    sf::RectangleShape card({pw, ph});
    card.setPosition(upgradePopupPos);
    card.setFillColor(sf::Color(18, 22, 44, 230));
    card.setOutlineColor(sf::Color(80, 120, 200));
    card.setOutlineThickness(1.5f);
    window.draw(card);

    float px = upgradePopupPos.x + 6.f;
    float py = upgradePopupPos.y + 5.f;

    drawText(tname + " Lv" + std::to_string(lv) + "/" + std::to_string(Tower::MAX_LEVEL),
             px, py, 13u, sf::Color(200, 220, 255));
    drawText("DMG:   " + std::to_string(hoveredTower->getDamage()),
             px, py + 18.f, 11u, sf::Color(200,200,200));
    drawText("Range: " + std::to_string((int)hoveredTower->getRange()),
             px, py + 32.f, 11u, sf::Color(200,200,200));
    if (hoveredTower->isSlow())
        drawText("Slow:  " + std::to_string((int)(hoveredTower->getSlowFactor()*100)) + "%",
                 px, py + 46.f, 11u, sf::Color(80,220,255));

    if (lv < Tower::MAX_LEVEL) {
        int ucost = Tower::upgradeCost(lv);
        bool canAff = economy.getMoney() >= ucost;
        // Button position already set in processEvents; just sync size
        btnUpgrade.rect.setSize({140.f, 36.f});
        sf::Color uc = canAff ? sf::Color(30, 140, 60) : sf::Color(80, 40, 40);
        sf::Color tc = canAff ? sf::Color::White : sf::Color(120, 80, 80);
        drawButton(btnUpgrade, uc, tc);
        btnUpgrade.enabled = canAff;
    } else {
        drawText("MAX LEVEL", px + 20.f, py + 66.f, 14u, sf::Color(255, 200, 0));
        btnUpgrade.enabled = false;
    }
}

// ─── renderGameOver / Victory / Paused ───────────────────────────────────────

void GameEngine::renderGameOver() {
    sf::RectangleShape ov({1280.f, 768.f});
    ov.setFillColor(sf::Color(0,0,0,170));
    window.draw(ov);

    drawCentred("GAME OVER", 640.f, 120.f, 60u, sf::Color(220,60,60));
    drawCentred("Score: "  + std::to_string(economy.getScore()),
                640.f, 210.f, 30u, sf::Color(255,220,50));
    drawCentred("Kills: "  + std::to_string(economy.getKillCount()),
                640.f, 252.f, 24u, sf::Color(200,200,200));
    drawCentred("Wave reached: " + std::to_string(waveManager->getWaveNumber()),
                640.f, 286.f, 22u, sf::Color(150,200,255));

    drawCentred("── High Scores ──", 640.f, 330.f, 20u, sf::Color(255,200,0));
    const auto& top = highScore.getTopScores();
    for (int i = 0; i < (int)top.size(); ++i)
        drawCentred("#" + std::to_string(i+1) + "  " + std::to_string(top[i]),
                    640.f, 356.f + i * 26.f, 18u);

    // Buttons below all content
    btnStart.rect.setPosition({440.f, 490.f});
    btnQuit.rect.setPosition( {440.f, 568.f});
    drawButton(btnStart, sf::Color(30,80,160));
    drawButton(btnQuit,  sf::Color(80,30,30));
}

void GameEngine::renderVictory() {
    sf::RectangleShape ov({1280.f, 768.f});
    ov.setFillColor(sf::Color(0,20,0,180));
    window.draw(ov);

    // Title
    drawCentred("VICTORY!", 640.f, 140.f, 72u, sf::Color(80,220,80));
    drawCentred("All 6 waves cleared!", 640.f, 230.f, 26u, sf::Color(180,255,180));

    // Stats block
    drawCentred("Final Score: " + std::to_string(economy.getScore()),
                640.f, 285.f, 28u, sf::Color(255,220,50));
    drawCentred("Kills: " + std::to_string(economy.getKillCount()),
                640.f, 325.f, 22u, sf::Color(200,200,200));

    // High score notice
    if (highScore.getRank(economy.getScore()) == 1)
        drawCentred("★ NEW HIGH SCORE ★", 640.f, 370.f, 26u, sf::Color(255,220,0));

    // Buttons appear after 2 seconds — positioned well below all text
    const float BUTTON_DELAY = 2.f;
    if (victoryTimer >= BUTTON_DELAY) {
        // Fade in
        float alpha = std::min(1.f, (victoryTimer - BUTTON_DELAY) / 0.5f);
        uint8_t a = (uint8_t)(alpha * 255.f);

        // Reposition buttons for this screen (below all text, centred)
        btnStart.rect.setPosition({440.f, 460.f});
        btnQuit.rect.setPosition( {440.f, 540.f});

        drawButton(btnStart, sf::Color(30,80,160,a),  sf::Color(255,255,255,a));
        drawButton(btnQuit,  sf::Color(80,30,30,a),   sf::Color(255,255,255,a));

        if (alpha < 1.f)
            drawCentred("...", 640.f, 430.f, 20u, sf::Color(120,120,120,a));
    } else {
        // Show a "press any key" hint with pulsing dots while waiting
        float pulse = std::sin(victoryTimer * 4.f) * 0.5f + 0.5f;
        uint8_t pa = (uint8_t)(pulse * 180.f + 40.f);
        drawCentred("Get ready...", 640.f, 460.f, 20u, sf::Color(120,180,120,pa));
    }
}

void GameEngine::renderPaused() {
    sf::RectangleShape ov({1280.f, 768.f});
    ov.setFillColor(sf::Color(0,0,0,150));
    window.draw(ov);

    drawCentred("PAUSED", 640.f, 240.f, 52u, sf::Color(180,200,255));
    drawCentred("Press ESC to resume", 640.f, 310.f, 22u, sf::Color(140,140,160));
    drawButton(btnResume, sf::Color(30,80,160));
    drawButton(btnMenu,   sf::Color(60,30,60));
}

// ─── Text + button helpers ────────────────────────────────────────────────────

void GameEngine::drawText(const std::string& str, float x, float y,
                           unsigned size, sf::Color col)
{
    if (!font) return;
    sf::Text t(*font, str, size);
    t.setFillColor(col);
    t.setPosition({x, y});
    window.draw(t);
}

void GameEngine::drawCentred(const std::string& str, float cx, float y,
                              unsigned size, sf::Color col)
{
    if (!font) return;
    sf::Text t(*font, str, size);
    t.setFillColor(col);
    auto b = t.getLocalBounds();
    t.setPosition({cx - b.size.x / 2.f, y});
    window.draw(t);
}

void GameEngine::drawButton(const UIButton& btn, sf::Color fill, sf::Color textCol) {
    sf::RectangleShape r = btn.rect;
    r.setFillColor(fill);
    r.setOutlineColor(sf::Color(80, 100, 160));
    r.setOutlineThickness(2.f);
    window.draw(r);
    if (!font) return;
    sf::Text t(*font, btn.label, 18u);
    t.setFillColor(textCol);
    auto b   = t.getLocalBounds();
    auto pos = btn.rect.getPosition();
    auto sz  = btn.rect.getSize();
    t.setPosition({pos.x + (sz.x - b.size.x) / 2.f,
                   pos.y + (sz.y - b.size.y) / 2.f - 4.f});
    window.draw(t);
}
