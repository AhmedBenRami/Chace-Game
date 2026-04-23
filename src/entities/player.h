// player.h
#pragma once

#include "entity.h"
#include "environment.h"
#include <algorithm>
#include <cmath>
using namespace std;

class Player : public Entity
{
public:
    Player(const char *dirPath, Vector2 startPos, Rectangle worldBounds, Font font,
           const char *runSoundPath,
           const char *jumpSoundPath,
           const char *damageSoundPath, const char *coinCollectSoundPath, int HP);
    ~Player();

    void update(float dt);
    void draw() override;

    void collideWithBlock(Rectangle block);
    void collideWithTeleporter();
    void collideWithEnemy(); // call once per overlapping enemy per frame

    void collideWithCoin();
    void collideWithProjectile();
    void collideWithGate(); // called when player touches the active gate
    void drawPlayerInfos();

    Rectangle getCoreBox() const; // returns collisionBox (for physics/collision)
    Rectangle getFullBox() const; // returns coreBox (full sprite frame, rarely needed)

    bool isFinishedLevel() const;
    bool isDead() const;
    bool hasReachedGate() const; // true when player touched the open gate
    void setFinishedLevel(bool v);
    void resetReachedGate(); // reset between levels

    void stopSound();
    void playSound();

private:
    Vector2 originalPosition;
    Vector2 velocity;
    float gravity;
    float jumpForce;
    float moveSpeed;
    Rectangle mapBorder;
    bool facingRight;
    bool onGround;
    Rectangle collisionBox; // smaller hit-box, centred on the sprite, used for all collision
    int health, originalHealth;
    int coinsCollected;
    float damageCooldown;
    float damageTimer;
    bool finishedLevel;
    bool reachedGate;
    Font hudFont;

    // Sounds  (entitySound inherited from Entity is the looped running footstep)
    Sound jumpSound;   // played once when the player leaves the ground
    Sound damageSound; // played once when the player takes a hit
    Sound coinCollectSound;

    void jump();
    void handleInput();
    void enforceWorldBounds();
};