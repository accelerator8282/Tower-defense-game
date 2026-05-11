#!/usr/bin/env bash
# build.sh — UCRT64 shell build
SFML_DIR="/ucrt64"
INC="-I${SFML_DIR}/include"
LIB="-L${SFML_DIR}/lib"
LIBS="-lsfml-graphics -lsfml-window -lsfml-system"
FLAGS="-std=c++17 -O2 -Wall"

SRCS="main.cpp GameEngine.cpp TileMap.cpp WaypointPath.cpp WaveManager.cpp \
      CombatSystem.cpp Enemy.cpp Tower.cpp Projectile.cpp \
      Economy.cpp HighScore.cpp"

echo "Compiling..."
g++ $FLAGS $INC $SRCS $LIB $LIBS -o TowerDefense.exe

if [ $? -eq 0 ]; then
    echo "Build successful -> TowerDefense.exe"
else
    echo "Build FAILED"
fi
