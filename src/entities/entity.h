#pragma once

#include "raylib.h"
#include <iostream>
#include <cmath>

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
    bool textureHasLoaded; // true once GPU textures have been uploaded from images

    // Raw CPU images — loaded immediately in constructor, used to create textures later
    Image idleImage, runningImage, jumpingImage, holoImage;
    Image singleImage; // used by non-player entities (enemy, boss)

    EntityState physicalState;
    Animation idleAnimation, runningAnimation, jumpingAnimation;
    Animation *currentAnimation;
    Texture2D holoTexture;

    // One shared sound slot:
    //   • Enemy  : plays while patrolling (looped footstep)
    //   • Boss   : plays each time a projectile is fired
    //   • Player : uses this slot for the running footstep (looped)
    Sound entitySound;

public:
    // Simplified constructors without mapBorders or ground levels
    Entity(const char *imagePath, Vector2 position);
    Entity(const char *dirPath, Vector2 position, bool isPlayer);

    virtual void update();
    virtual void playerUpdate();

    virtual void draw();

    virtual ~Entity();
};
