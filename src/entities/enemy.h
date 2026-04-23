#pragma once

#include "entity.h"

class Enemy : public Entity
{
public:
    // imagePath   : spritesheet (frames must be square, width = N * height)
    // position    : spawn position (top-left of the enemy)
    // worldBounds : rectangle the enemy patrols inside
    // speed       : horizontal patrol speed in px/s
    Enemy(const char *imagePath, Vector2 position, Rectangle worldBounds, float speed, const char *walkSoundPath);
    ~Enemy();

    void update(float dt);
    void draw() override;

    Rectangle getCoreBox() const;
    bool isAlive() const;
    void stopSound();
    void playSound();

private:
    Rectangle mapBorder;
    float moveSpeed;
    float dirX; // +1 = right, -1 = left
    bool alive;
};