# Platformer Quest

A 2D action platformer built with [Raylib](https://www.raylib.com/).  
Navigate 3 levels, dodge enemies, collect coins, survive the boss, and reach the gate.

---

## Controls

| Key | Action |
|-----|--------|
| `A` / `←` | Move left |
| `D` / `→` | Move right |
| `W` / `Space` / `↑` | Jump |
| `Enter` | Pause / Resume |

---

## Gameplay

- You have **3 HP**. Each enemy or projectile hit costs 1 HP (with a 2s invincibility window after each hit).
- A **Boss** waits at the right end of every level. You cannot advance past 100px from it until it is defeated.
- Once the boss is defeated the **Gate** unlocks — walk into it to advance to the next level.
- Each level has a **120-second timer**. Running out of time ends the game.
- **Teleporters** (`T` in the level file) send you back to the start position.

---

## Project Structure

```
.
├── main.cpp
├── CMakeLists.txt
├── assets/               # Textures, fonts, level files
├── Raylib/
│   ├── include/          # raylib.h and friends
│   ├── linux/            # libraylib.a  (Linux)
│   ├── macos/            # libraylib.a  (macOS)
│   └── windows/          # libraylib.a  (Windows / MinGW)
└── src/
    ├── entities/         # entity, player, enemy, boss
    ├── game/             # GameManager
    ├── map/              # Environment
    └── menu/             # MainMenu
```

---

## How to Build

### Requirements

- CMake 3.10 or newer
- A C++11-capable compiler:
  - **Linux** — GCC or Clang
  - **macOS** — Clang (Xcode Command Line Tools)
  - **Windows** — MinGW-w64 (the `libraylib.a` in `Raylib/windows/` must match your MinGW build)

### Linux / macOS

```bash
# 1. Clone or download the project, then enter the directory
cd platformer-quest

# 2. Configure
cmake -B build

# 3. Compile
cmake --build build

# 4. Run (executable is placed in the project root)
./GameProject
```

### Windows (MinGW)

```bash
# 1. Open a MinGW terminal and enter the project directory

# 2. Configure (tell CMake to use the MinGW Makefiles generator)
cmake -B build -G "MinGW Makefiles"

# 3. Compile
cmake --build build

# 4. Run
GameProject.exe
```

### Rebuilding after changes

You only need to re-run step 3 (`cmake --build build`) after editing source files.  
Re-run both steps 2 and 3 only if you add/remove files or change `CMakeLists.txt`.

### Clean build

```bash
rm -rf build/          # Linux / macOS
rmdir /s /q build      # Windows cmd
cmake -B build && cmake --build build
```

---

## Adding a New Level

1. Create `assets/levelN.txt` using the same tile format:
   - `#` — solid platform block
   - `C` — coin
   - `T` — teleporter
2. Update `GameManager::loadLevel()` in `src/game/GameManager.cpp` to reference the new file.
3. Increment `TOTAL_LEVELS` in `src/game/GameManager.hpp`.
