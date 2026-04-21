#pragma once

#include "raylib.h"
#include <iostream>

typedef enum
{
    IDLE,
    RUNNING,
    JUMPING
} EntityState;

typedef struct
{
    Texture2D tileTexture;
    Rectangle currentFrame;

    int firstIndex, currentIndex, lastIndex;

    float speed;
    float timeLeft;
} Animation;

class Entity
{
protected:
    Rectangle coreBox; // main rectangle to frame the object on screen
    bool isPlayer;

    EntityState physicalState;
    Animation idleAnimation, runningAnimation, jumpingAnimation;
    Animation *currentAnimation;

public:
    // Simplified constructors without mapBorders or ground levels
    Entity(const char *imagePath, Vector2 position);
    Entity(const char *dirPath, Vector2 position, bool isPlayer);

    virtual void update();
    virtual void playerUpdate();

    virtual void draw();

    virtual ~Entity();
};
