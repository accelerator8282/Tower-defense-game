@echo off
:: ============================================================
::  build.bat  —  UCRT64 (MSYS2) single-command build
::  Usage: double-click OR run from a UCRT64 terminal
::
::  One-time setup in UCRT64 terminal:
::      pacman -S mingw-w64-ucrt-x86_64-sfml
:: ============================================================

set SFML_DIR=C:/msys64/ucrt64
set INC=-I"%SFML_DIR%/include"
set LIB=-L"%SFML_DIR%/lib"
set LIBS=-lsfml-graphics -lsfml-window -lsfml-system
set FLAGS=-std=c++17 -O2 -Wall

set SRCS=main.cpp GameEngine.cpp TileMap.cpp WaypointPath.cpp WaveManager.cpp ^
         CombatSystem.cpp Enemy.cpp Tower.cpp Projectile.cpp ^
         Economy.cpp HighScore.cpp

echo Compiling Tower Defense...
g++ %FLAGS% %INC% %SRCS% %LIB% %LIBS% -o TowerDefense.exe

if %ERRORLEVEL% == 0 (
    echo.
    echo  Build successful ^-^> TowerDefense.exe
    echo  Make sure arial.ttf is in the same folder.
    echo.
) else (
    echo.
    echo  Build FAILED. See errors above.
    echo.
    pause
)
