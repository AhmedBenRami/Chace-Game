#include "enemy.h"

Enemy::Enemy(const char *imagePath, Vector2 position, Rectangle worldBounds, float speed, const char *walkSoundPath)
    : Entity(imagePath, position),
      mapBorder(worldBounds), moveSpeed(speed), dirX(1.0f), alive(true)
{
    // Snap to world floor on spawn
    float groundY = mapBorder.y + mapBorder.height - coreBox.height;
    coreBox.y = groundY;

    coreBox.height = currentAnimation->currentFrame.height / 2;
    coreBox.y += coreBox.height;

    // entitySound = looped patrol footstep
    if (walkSoundPath)
        entitySound = LoadSound(walkSoundPath);
}

void Enemy::update(float dt)
{
    if (!alive)
    {
        if (IsSoundPlaying(entitySound))
            StopSound(entitySound);
        return;
    }

    // ── Patrol footstep: loop while alive ────────────────────────────────────
    if (!IsSoundPlaying(entitySound))
        PlaySound(entitySound);

    // Move horizontally
    coreBox.x += moveSpeed * dirX * dt;

    // Reverse at world borders
    if (coreBox.x <= mapBorder.x)
    {
        coreBox.x = mapBorder.x;
        dirX = 1.0f;
    }
    if (coreBox.x + coreBox.width >= mapBorder.x + mapBorder.width)
    {
        coreBox.x = mapBorder.x + mapBorder.width - coreBox.width;
        dirX = -1.0f;
    }

    // Delegate frame advancement to Entity
    Entity::update();
}

void Enemy::draw()
{
    if (!alive)
        return;

    // Source rect: bottom half of each frame (top half is empty padding)
    Rectangle src = currentAnimation->currentFrame;
    float halfH = src.height / 2.0f;
    src.y += halfH; // start from the vertical mid-point of the frame
    src.height = halfH;

    if (dirX < 0)
        src.width = -src.width; // negative width = horizontal flip in DrawTexturePro

    // Destination: same coreBox (the half-height sprite fills the full dest rect)
    DrawTexturePro(currentAnimation->tileTexture, src, coreBox, {0, 0}, 0, WHITE);
}

Rectangle Enemy::getCoreBox() const { return coreBox; }
bool Enemy::isAlive() const { return alive; }

void Enemy::stopSound()
{
    if (IsSoundPlaying(entitySound))
        StopSound(entitySound);
}
void Enemy::playSound()
{
    if (!IsSoundPlaying(entitySound))
        PlaySound(entitySound);
}
Enemy::~Enemy()
{
    // Stop playback before ~Entity() calls UnloadSound(entitySound)
    if (IsSoundPlaying(entitySound))
        StopSound(entitySound);
    // entitySound itself is unloaded by ~Entity()
}