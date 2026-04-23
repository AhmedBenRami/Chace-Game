#include "GameManager.hpp"

GameManager::GameManager(int width, int height, const char *title, int fps,
                         const char *winSoundPath, const char *loseSoundPath)
{
    windowSize = {(float)width, (float)height};
    InitWindow(width, height, title);
    SetTargetFPS(fps);
    InitAudioDevice();

    globalFont = LoadFont("assets/font.ttf");
    loadingBackground = LoadTexture("assets/loading_bg.png");

    mainMenu = new MainMenu(width, height, globalFont, "assets/menu_music.mp3");

    mainMenu->onStart = [this]()
    { setState(LOADING); };
    mainMenu->onExit = [this]()
    { setState(EXIT); };

    gameState = MENU;
    boss = nullptr;
    player = nullptr;
    map = nullptr;
    loadingThread = nullptr;
    loadingDone = false;
    currentLevel = 1;
    levelTimer = LEVEL_TIME_LIMIT;
    levelTimerMax = LEVEL_TIME_LIMIT;

    winSound = {0};
    loseSound = {0};
    if (winSoundPath)
        winSound = LoadSound(winSoundPath);
    if (loseSoundPath)
        loseSound = LoadSound(loseSoundPath);

    camera.target = {0, 0};
    camera.offset = {0, windowSize.y / 2};
    camera.rotation = 0;
    camera.zoom = 1;
}

GameManager::~GameManager()
{
    // Make sure the loading thread has finished before tearing down
    if (loadingThread)
    {
        if (loadingThread->joinable())
            loadingThread->join();
        delete loadingThread;
        loadingThread = nullptr;
    }
    delete mainMenu;
    delete boss;
    delete player;
    delete map;
    for (Enemy *e : enemies)
        delete e;
    enemies.clear();
    UnloadFont(globalFont);
    UnloadTexture(loadingBackground);
    UnloadSound(winSound);
    UnloadSound(loseSound);
    CloseAudioDevice();
    CloseWindow();
}

GameState GameManager::getState() const { return gameState; }
void GameManager::setState(GameState newState) { gameState = newState; }
bool GameManager::isRunning() const { return !WindowShouldClose() && gameState != EXIT; }

void GameManager::update(float deltaTime)
{
    switch (gameState)
    {
    case MENU:
        mainMenu->playMusic();
        mainMenu->Update();
        break;

    case LOADING:
        // Spawn the loading thread once — it does disk I/O + object construction.
        // GPU texture uploads happen lazily on the main thread inside each object's update().
        mainMenu->stopMusic();
        if (!loadingThread)
        {
            unloadComponents();
            loadingDone = false;
            loadingThread = new thread(&GameManager::loadComponents, this);
            loadingThread->detach();
        }
        // Once the background thread signals it's done, clean up and start playing
        if (loadingDone.load())
        {
            delete loadingThread;
            loadingThread = nullptr;
            loadingDone = false;
            // gameState is already set to PLAYING inside loadLevel()
        }
        break;

    case PLAYING:
    {
        player->playSound();
        for (Enemy *e : enemies)
            e->playSound();
        // ── PAUSE toggle ─────────────────────────────────────────────────────
        if (IsKeyPressed(KEY_ENTER))
        {
            setState(PAUSED);
            break;
        }

        // ── Level timer ──────────────────────────────────────────────────────
        levelTimer -= deltaTime;
        if (levelTimer <= 0.0f)
        {
            levelTimer = 0.0f;
            if (loseSound.frameCount > 0)
                PlaySound(loseSound);
            setState(LOSE);
            break;
        }

        map->update();
        player->update(deltaTime);

        // ── Teleporters ───────────────────────────────────────────────────────
        for (const Rectangle &teleporter : map->getTeleporters().rects)
        {
            if (CheckCollisionRecs(player->getCoreBox(), teleporter))
                player->collideWithTeleporter();
        }

        // ── Enemies ───────────────────────────────────────────────────────────
        for (Enemy *enemy : enemies)
        {
            enemy->update(deltaTime);
            if (CheckCollisionRecs(player->getCoreBox(), enemy->getCoreBox()))
                player->collideWithEnemy();
        }

        // ── Boss ──────────────────────────────────────────────────────────────
        if (boss && !boss->isDefeated())
        {
            boss->update(deltaTime, player->getCoreBox().x);

            // Projectile → player collision
            for (const Projectile &p : boss->getProjectiles())
            {
                if (p.active && CheckCollisionRecs(player->getCoreBox(), p.rect))
                    player->collideWithProjectile();
            }

            // Boss barrier: invisible wall 100 px in front of the boss's left edge.
            // Spans the full world height so the player cannot jump over it.
            static constexpr float BOSS_STOP_DIST = 100.0f;
            Rectangle bossBox = boss->getCoreBox();
            float wallX = bossBox.x - BOSS_STOP_DIST;
            Rectangle wb2 = map->getWorldBounds();
            Rectangle barrier = {wallX, wb2.y, 1.0f, wb2.height};
            Rectangle pb = player->getCoreBox();
            if (pb.x + pb.width > wallX)
                player->collideWithBlock(barrier);

            // Boss just defeated → unlock gate
            if (boss->isDefeated())
            {
                player->setFinishedLevel(true);
                map->activateGate();
            }
        }

        // ── Gate collision (only when active / boss defeated) ─────────────────
        if (map->getGate().active &&
            CheckCollisionRecs(player->getCoreBox(), map->getGate().rect))
        {
            player->collideWithGate();
        }

        // ── Player reached gate → next level or WIN ───────────────────────────
        if (player->hasReachedGate())
        {
            if (currentLevel < TOTAL_LEVELS)
            {
                currentLevel++;
                setState(LOADING); // triggers unload + loadLevel(currentLevel)
            }
            else
            {
                if (winSound.frameCount > 0)
                    PlaySound(winSound);
                setState(WIN);
            }
            break;
        }

        // ── Player dead ───────────────────────────────────────────────────────
        if (player->isDead())
        {
            if (loseSound.frameCount > 0)
                PlaySound(loseSound);
            setState(LOSE);
            break;
        }

        // ── Block collisions ──────────────────────────────────────────────────
        for (const Rectangle &block : map->getBlocks().rects)
        {
            if (CheckCollisionRecs(player->getCoreBox(), block))
                player->collideWithBlock(block);
        }

        // ── Coin collisions ───────────────────────────────────────────────────
        for (const Rectangle coin : map->getCoins().coinsRec)
        {
            if (CheckCollisionRecs(player->getCoreBox(), coin))
            {
                player->collideWithCoin();
                map->collectCoin(coin);
            }
        }

        // ── Camera follows player ─────────────────────────────────────────────
        camera.target = {
            (player->getCoreBox().x + map->getWorldBounds().x) / 2,
            (player->getCoreBox().y + player->getCoreBox().height) / 2};
        break;
    }

    case PAUSED:
        player->stopSound();
        for (Enemy *e : enemies)
            e->stopSound();

        if (IsKeyPressed(KEY_ENTER))
            setState(PLAYING);
        break;

    case WIN:
        if (IsKeyPressed(KEY_ENTER))
        {
            currentLevel = 1;
            setState(MENU);
        }
        player->stopSound();
        for (Enemy *e : enemies)
            e->stopSound();
        break;

    case LOSE:
        if (IsKeyPressed(KEY_ENTER))
        {
            currentLevel = 1;
            setState(MENU);
        }
        player->stopSound();
        for (Enemy *e : enemies)
            e->stopSound();

        break;

    case EXIT:
        break;
    }

    // repeatedly update window size
    windowSize = {(float)GetScreenWidth(), (float)GetScreenHeight()};
}

void GameManager::draw() const
{
    // BeginTextureMode(canvas);
    // ClearBackground(BLACK);
    BeginDrawing();

    switch (gameState)
    {
    case MENU:
        mainMenu->Draw();
        break;

    case LOADING:
        drawLoading();
        break;

    case PLAYING:
    {
        map->drawBg();

        // ── HUD ──────────────────────────────────────────────────────────────
        player->drawPlayerInfos(); // coins + health on the left

        // Level indicator (top centre)
        {
            const char *lvlText = TextFormat("Level %d X %d", currentLevel, TOTAL_LEVELS);
            Vector2 lvlSize = MeasureTextEx(globalFont, lvlText, 20, 1);
            DrawTextEx(globalFont, lvlText,
                       {GetScreenWidth() / 2.0f - lvlSize.x / 2.0f, 20}, 20, 1, WHITE);
        }

        // Timer on the right (turns red when under 10 s)
        {
            Color timerColor = (levelTimer <= 10.0f) ? RED : WHITE;
            DrawTextEx(globalFont, TextFormat("Time: %d", (int)levelTimer),
                       {(float)GetScreenWidth() - 150, 20}, 20, 1, timerColor);
        }

        // ── World ─────────────────────────────────────────────────────────────
        BeginMode2D(camera);
        map->draw();
        player->draw();
        for (Enemy *enemy : enemies)
            enemy->draw();
        if (boss)
            boss->draw();
        EndMode2D();
        break;
    }

    case PAUSED:
    {
        // Re-draw the last playing frame underneath, then dim it
        map->drawBg();
        BeginMode2D(camera);
        map->draw();
        player->draw();
        for (Enemy *enemy : enemies)
            enemy->draw();
        if (boss)
            boss->draw();
        EndMode2D();

        // Semi-transparent dark overlay
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 127});

        // Pause text
        {
            const char *pt = "PAUSED";
            float ptSize = 60.0f;
            Vector2 ptS = MeasureTextEx(globalFont, pt, ptSize, 1);
            DrawTextEx(globalFont, pt,
                       {GetScreenWidth() / 2.0f - ptS.x / 2.0f, GetScreenHeight() / 2.0f - 40}, ptSize, 1, WHITE);
            const char *sub = "Press ENTER to resume";
            Vector2 subS = MeasureTextEx(globalFont, sub, 20, 1);
            DrawTextEx(globalFont, sub,
                       {GetScreenWidth() / 2.0f - subS.x / 2.0f, GetScreenHeight() / 2.0f + 35}, 20, 1, LIGHTGRAY);
        }
        break;
    }

    case WIN:
    {
        ClearBackground(DARKGREEN);
        const char *w1 = "YOU WIN!";
        const char *w2 = TextFormat("You completed all %d levels!", TOTAL_LEVELS);
        const char *w3 = "Press ENTER to return to menu";
        Vector2 s1 = MeasureTextEx(globalFont, w1, 60, 1);
        Vector2 s2 = MeasureTextEx(globalFont, w2, 24, 1);
        Vector2 s3 = MeasureTextEx(globalFont, w3, 20, 1);
        DrawTextEx(globalFont, w1, {GetScreenWidth() / 2.0f - s1.x / 2.0f, GetScreenHeight() / 2.0f - 50}, 60, 1, WHITE);
        DrawTextEx(globalFont, w2, {GetScreenWidth() / 2.0f - s2.x / 2.0f, GetScreenHeight() / 2.0f + 20}, 24, 1, LIGHTGRAY);
        DrawTextEx(globalFont, w3, {GetScreenWidth() / 2.0f - s3.x / 2.0f, GetScreenHeight() / 2.0f + 60}, 20, 1, LIGHTGRAY);
        break;
    }

    case LOSE:
    {
        ClearBackground(MAROON);
        const char *l1 = "GAME OVER";
        const char *l2 = (levelTimer <= 0.0f) ? "Time's up!" : "You were defeated.";
        const char *l3 = "Press ENTER to return to menu";
        Vector2 ls1 = MeasureTextEx(globalFont, l1, 60, 1);
        Vector2 ls2 = MeasureTextEx(globalFont, l2, 24, 1);
        Vector2 ls3 = MeasureTextEx(globalFont, l3, 20, 1);
        DrawTextEx(globalFont, l1, {GetScreenWidth() / 2.0f - ls1.x / 2.0f, GetScreenHeight() / 2.0f - 50}, 60, 1, WHITE);
        DrawTextEx(globalFont, l2, {GetScreenWidth() / 2.0f - ls2.x / 2.0f, GetScreenHeight() / 2.0f + 20}, 24, 1, LIGHTGRAY);
        DrawTextEx(globalFont, l3, {GetScreenWidth() / 2.0f - ls3.x / 2.0f, GetScreenHeight() / 2.0f + 60}, 20, 1, LIGHTGRAY);
        break;
    }

    case EXIT:
        break;
    }

    EndDrawing();
}

void GameManager::unloadComponents()
{
    delete boss;
    boss = nullptr;
    delete player;
    player = nullptr;
    delete map;
    map = nullptr;
    for (Enemy *e : enemies)
        delete e;
    enemies.clear();
}

void GameManager::loadComponents()
{
    // Artificial delay so the loading screen is visible (runs on background thread)
    this_thread::sleep_for(chrono::seconds(3));

    loadLevel(currentLevel);

    // Signal the main thread that loading has finished
    loadingDone = true;
}

void GameManager::loadLevel(int level)
{
    // ── Map (same assets, different level file) ───────────────────────────────
    const char *levelFile = (level == 1)   ? "assets/level1.txt"
                            : (level == 2) ? "assets/level2.txt"
                                           : "assets/level3.txt";

    map = new Environment(levelFile,
                          "assets/map_bg.png",
                          "assets/ground.png",
                          "assets/map_block.png",
                          "assets/coin.png",
                          "assets/tree.png",
                          "assets/teleporter.png",
                          "assets/gate.png");

    // ── Player ────────────────────────────────────────────────────────────────
    player = new Player("assets", {100, 100}, map->getWorldBounds(), globalFont,
                        "assets/player_run.mp3",
                        "assets/player_jump.mp3",
                        "assets/player_damage.mp3",
                        "assets/player_coinCollect.mp3", 10);
    player->resetReachedGate();

    Rectangle wb = map->getWorldBounds();

    // ── Enemies — scale count and speed with level ────────────────────────────
    if (level == 1)
    {
        enemies.push_back(new Enemy("assets/enemy.png", {400, 0}, wb, 120.0f, "assets/enemy_walk.mp3"));
        enemies.push_back(new Enemy("assets/enemy.png", {700, 0}, wb, 90.0f, "assets/enemy_walk.mp3"));
    }
    else if (level == 2)
    {
        enemies.push_back(new Enemy("assets/enemy.png", {300, 0}, wb, 140.0f, "assets/enemy_walk.mp3"));
        enemies.push_back(new Enemy("assets/enemy.png", {600, 0}, wb, 120.0f, "assets/enemy_walk.mp3"));
        enemies.push_back(new Enemy("assets/enemy.png", {900, 0}, wb, 110.0f, "assets/enemy_walk.mp3"));
        enemies.push_back(new Enemy("assets/enemy.png", {1200, 0}, wb, 130.0f, "assets/enemy_walk.mp3"));
    }
    else // level 3
    {
        enemies.push_back(new Enemy("assets/enemy.png", {300, 0}, wb, 160.0f, "assets/enemy_walk.mp3"));
        enemies.push_back(new Enemy("assets/enemy.png", {600, 0}, wb, 150.0f, "assets/enemy_walk.mp3"));
        enemies.push_back(new Enemy("assets/enemy.png", {900, 0}, wb, 140.0f, "assets/enemy_walk.mp3"));
        enemies.push_back(new Enemy("assets/enemy.png", {1200, 0}, wb, 170.0f, "assets/enemy_walk.mp3"));
        enemies.push_back(new Enemy("assets/enemy.png", {1500, 0}, wb, 155.0f, "assets/enemy_walk.mp3"));
        enemies.push_back(new Enemy("assets/enemy.png", {1800, 0}, wb, 145.0f, "assets/enemy_walk.mp3"));
    }

    // ── Boss — more projectiles each level ────────────────────────────────────
    int bossShots = (level == 1) ? 5 : (level == 2) ? 10
                                                    : 20;
    boss = new Boss("assets/boss.png",
                    "assets/projectile.png",
                    wb,
                    bossShots,
                    "assets/boss_fire.mp3");

    // ── Reset timer ───────────────────────────────────────────────────────────
    levelTimer = LEVEL_TIME_LIMIT;

    gameState = PLAYING;
}

void GameManager::drawLoading() const
{
    DrawTexturePro(
        loadingBackground,
        {0, 0, (float)loadingBackground.width, (float)loadingBackground.height},
        {0, 0, (float)windowSize.x, (float)windowSize.y},
        {0, 0}, 0.0f, WHITE);

    // Draw "Loading..." text at bottom right
    const char *text = "Loading...";
    float fontSize = 32.0f;
    float spacing = 2.0f;
    Vector2 textSize = MeasureTextEx(globalFont, text, fontSize, spacing);
    float padding = 20.0f;
    Vector2 position = {
        (windowSize.x - textSize.x) / 2,
        windowSize.y - textSize.y - padding};
    // Vector2 position = {0, 0};
    DrawTextEx(globalFont, text, position, fontSize, spacing, WHITE);

    // this_thread::sleep_for(chrono::seconds(3));
    // WaitTime(3);
}