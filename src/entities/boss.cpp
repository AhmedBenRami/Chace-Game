// boss.cpp
#include "boss.h"
#include <cmath>
#include <iostream>
using namespace std;

// ── helper: initialise an Animation from a loaded texture ──────────────────
static void InitAnim(Animation &anim, Texture2D tex)
{
    float size = (float)tex.height;
    anim.tileTexture = tex;
    anim.currentFrame = {0, 0, size, size};
    anim.firstIndex = 0;
    anim.currentIndex = 0;
    anim.lastIndex = (int)(tex.width / tex.height);
    anim.speed = 0.12f;
    anim.timeLeft = 0.12f;
}

// ── constructor ─────────────────────────────────────────────────────────────
Boss::Boss(const char *imagePath,
           const char *projectilePath,
           Rectangle worldBounds,
           int maxProjectiles,
           const char *fireSoundPath)
    : Entity(imagePath, {0, 0}), // position fixed up below
      mapBorder(worldBounds),
      maxShots(maxProjectiles),
      shotsFired(0),
      shootCooldown(SHOOT_COOLDOWN),
      shootTimer(SHOOT_COOLDOWN),
      aggroed(false),
      defeated(false),
      textureHasLoaded(false)
{
    // Place boss on the right edge of the world, snapped to the floor
    coreBox.x = mapBorder.x + mapBorder.width - coreBox.width - 10.0f;
    coreBox.y = mapBorder.y + mapBorder.height - coreBox.height;

    // entitySound = single-shot fire sound played each time a projectile is spawned
    if (fireSoundPath)
        entitySound = LoadSound(fireSoundPath);

    projectileImage = LoadImage(projectilePath);
    if (projectileImage.data == nullptr)
    {
        cout << "Error loading projectile image: " << projectilePath << endl;
        exit(6);
    }
}

// ── destructor ──────────────────────────────────────────────────────────────
Boss::~Boss()
{
    if (textureHasLoaded)
        UnloadTexture(projectileTexture);
    else
        UnloadImage(projectileImage);
    // Entity destructor handles the boss body texture / image via its own flag
}

// ── update ──────────────────────────────────────────────────────────────────
void Boss::update(float dt, float playerX)
{
    if (defeated)
        return;

    // ── Lazy texture upload (runs once on the main thread) ───────────────────
    if (!textureHasLoaded)
    {
        projectileTexture = LoadTextureFromImage(projectileImage);
        UnloadImage(projectileImage);
        textureHasLoaded = true;
    }

    // ── check aggro ─────────────────────────────────────────────────────────
    float dist = fabsf(playerX - (coreBox.x + coreBox.width / 2.0f));
    if (!aggroed && dist <= TRIGGER_DISTANCE)
        aggroed = true;

    // ── shoot logic ─────────────────────────────────────────────────────────
    if (aggroed && shotsFired < maxShots)
    {
        shootTimer -= dt;
        if (shootTimer <= 0.0f)
        {
            spawnProjectile(playerX);
            shootTimer = shootCooldown;
        }
    }

    // ── check defeat condition ───────────────────────────────────────────────
    if (shotsFired >= maxShots)
        defeated = true;

    // ── advance boss body animation ──────────────────────────────────────────
    Entity::update(); // uses GetFrameTime() internally — fine for raylib

    // ── advance projectile animations + lifetime ─────────────────────────────
    for (Projectile &p : projectiles)
    {
        if (!p.active)
            continue;

        p.rect.x += p.velocityX * dt;
        p.lifetime -= dt;

        if (p.lifetime <= 0.0f)
        {
            p.active = false;
            continue;
        }

        // Kill if it leaves the world bounds
        if (p.rect.x + p.rect.width < mapBorder.x ||
            p.rect.x > mapBorder.x + mapBorder.width)
        {
            p.active = false;
            continue;
        }

        advanceAnimation(p.animation, dt);
    }
}

// ── draw ─────────────────────────────────────────────────────────────────────
void Boss::draw()
{
    if (defeated)
        return;

    // Draw boss body (always faces left toward player)
    Rectangle src = currentAnimation->currentFrame;
    src.width = -src.width; // flip horizontally
    DrawTexturePro(currentAnimation->tileTexture, src, coreBox, {0, 0}, 0, WHITE);

    // Draw active projectiles
    for (Projectile &p : projectiles)
    {
        if (!p.active)
            continue;
        p.animation.currentFrame.width *= -1;
        DrawTexturePro(p.animation.tileTexture,
                       p.animation.currentFrame,
                       p.rect, {0, 0}, 0, WHITE);
        p.animation.currentFrame.width *= -1;
    }
}

// ── public accessors ─────────────────────────────────────────────────────────
bool Boss::isDefeated() const { return defeated; }
Rectangle Boss::getCoreBox() const { return coreBox; }

const vector<Projectile> &Boss::getProjectiles() const { return projectiles; }

// ── private helpers ──────────────────────────────────────────────────────────
void Boss::spawnProjectile(float playerX)
{
    float projSize = (float)projectileTexture.height;

    Projectile p;
    // Spawn from the left edge of the boss, vertically centered
    p.rect = {coreBox.x - projSize,
              coreBox.y + coreBox.height / 2.0f - projSize / 2.0f,
              projSize, projSize};
    p.lifetime = PROJECTILE_LIFE;
    p.active = true;

    // Always shoot toward the player
    p.velocityX = (playerX < coreBox.x) ? -PROJECTILE_SPEED : PROJECTILE_SPEED;

    InitAnim(p.animation, projectileTexture);

    projectiles.push_back(p);
    ++shotsFired;
    PlaySound(entitySound); // single-shot fire sound
}

void Boss::advanceAnimation(Animation &anim, float dt)
{
    anim.timeLeft -= dt;
    if (anim.timeLeft <= 0.0f)
    {
        anim.currentIndex = (anim.currentIndex >= anim.lastIndex - 1)
                                ? 0
                                : anim.currentIndex + 1;
        anim.currentFrame.x = (float)anim.currentIndex * anim.currentFrame.width;
        anim.timeLeft = anim.speed;
    }
}