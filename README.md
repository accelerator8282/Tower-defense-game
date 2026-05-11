# Tower Defense — SFML 3 / UCRT64 Build Guide

## Prerequisites

Install [MSYS2](https://www.msys2.org/), then open the **UCRT64** terminal and run:

```bash
pacman -S mingw-w64-ucrt-x86_64-sfml
```

This installs SFML 3 (graphics, window, system) to `C:/msys64/ucrt64/`.

---

## Building

### Option A — double-click (Windows Explorer)
Double-click **`build.bat`** from the project folder.

### Option B — UCRT64 shell
```bash
cd /path/to/tower-defense-refactored
bash build.sh
```

Both produce **`TowerDefense.exe`** in the same folder.

---

## Running

```bash
./TowerDefense.exe
```

Or double-click `TowerDefense.exe`.  
**Important:** the UCRT64 DLLs must be on PATH. If you launch from outside
the UCRT64 shell, copy these DLLs next to the exe:

```
C:/msys64/ucrt64/bin/sfml-graphics-3.dll
C:/msys64/ucrt64/bin/sfml-window-3.dll
C:/msys64/ucrt64/bin/sfml-system-3.dll
C:/msys64/ucrt64/bin/libstdc++-6.dll
C:/msys64/ucrt64/bin/libgcc_s_seh-1.dll
C:/msys64/ucrt64/bin/libwinpthread-1.dll
```

---

## Optional — HUD text

Place any `.ttf` font file named **`arial.ttf`** next to the exe.  
The game runs without it; only the money/lives/score/wave labels are hidden.

---

## Controls

| Input | Action |
|---|---|
| **Left-click** a green (grass) tile | Place a tower ($50) |
| Close button | Quit |

## Gameplay

- 6 waves of enemies follow an S-curve path across the map.
- Enemies that reach the exit cost you a **life** (20 total).
- Enemies killed by towers earn **money** and **score**.
- Between waves you receive a wave-clear **bonus**.
- Game ends when lives reach 0 (Game Over) or all 6 waves are cleared (Victory).

## Enemy types

| Type | Color | HP | Speed | Reward |
|---|---|---|---|---|
| Basic | Red | 100 | 80 px/s | $10 |
| Fast | Orange | 60 | 160 px/s | $15 |
| Tank | Purple | 300 | 40 px/s | $30 |
