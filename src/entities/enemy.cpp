// enemy.cpp
#include "enemy.h"

Enemy::Enemy(const char *imagePath, Vector2 position, Rectangle worldBounds, float speed)
    : Entity(imagePath, position), // Entity loads the texture and sets up coreBox + animation
      mapBorder(worldBounds), moveSpeed(speed), dirX(1.0f), alive(true)
{
    // Snap to world floor on spawn
    float groundY = mapBorder.y + mapBorder.height - coreBox.height;
    coreBox.y = groundY;

    coreBox.height = currentAnimation->currentFrame.height / 2;
    coreBox.y += coreBox.height;
}

void Enemy::update(float dt)
{
    if (!alive)
        return;

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