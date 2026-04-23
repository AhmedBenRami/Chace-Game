#pragma once
#include "raylib.h"
#include "MainMenu.h"
#include "environment.h"
#include "player.h"
#include "enemy.h"
#include "boss.h"
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <iomanip>

#define TOTAL_LEVELS 3
#define LEVEL_TIME_LIMIT 120.0

using namespace std;

typedef enum
{
    MENU,
    LOADING,
    PLAYING,
    PAUSED,
    WIN,
    LOSE,
    EXIT
} GameState;

class GameManager
{
public:
    GameManager(int width, int height, const char *title, int fps,
                const char *winSoundPath,
                const char *loseSoundPath);
    ~GameManager();

    void update(float deltaTime);
    void draw() const;
    void drawLoading() const;
    void loadComponents();
    void unloadComponents();
    void loadLevel(int level); // 1, 2, or 3

    GameState getState() const;
    void setState(GameState newState);
    bool isRunning() const;

private:
    Vector2 windowSize;
    GameState gameState;
    Font globalFont;
    Texture2D loadingBackground;

    Camera2D camera;
    MainMenu *mainMenu;
    Player *player;
    Environment *map;
    Boss *boss;
    vector<Enemy *> enemies;

    int currentLevel;         // 1, 2, or 3
    float levelTimer;         // counts DOWN from levelTimerMax
    float levelTimerMax;      // seconds per level (e.g. 120)
    thread *loadingThread;    // background thread used during LOADING state
    atomic<bool> loadingDone; // set to true by the thread when loadLevel() finishes

    Sound winSound;  // played once when the player wins the game
    Sound loseSound; // played once when the player loses
};