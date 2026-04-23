#pragma once

#include "entity.h"

class Enemy : public Entity
{
public:

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
