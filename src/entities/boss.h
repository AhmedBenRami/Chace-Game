#pragma once

#include "entity.h"
#include <vector>
using namespace std;

#define TRIGGER_DISTANCE 500.0 // px — boss wakes up
#define PROJECTILE_SPEED 400.0 // px/s
#define PROJECTILE_LIFE 3.0    // seconds before auto-despawn
#define SHOOT_COOLDOWN 1.2     // seconds between shots

// An animated projectile fired by the boss
struct Projectile
{
    Rectangle rect;      // position + size in world space
    Animation animation; // animated sprite (uses Entity's Animation struct)
    float lifetime;      // seconds remaining before this projectile dies
    float velocityX;     // horizontal speed (negative = moving left toward player)
    bool active;         // false once lifetime expires
};

class Boss : public Entity
{
public:
    // imagePath       : boss spritesheet
    // projectilePath  : projectile spritesheet
    // worldBounds     : the full map rectangle (boss snaps to its right edge)
    // maxProjectiles  : how many shots before the boss disappears (default 20)
    Boss(const char *imagePath,
         const char *projectilePath,
         Rectangle worldBounds,
         int maxProjectiles,
         const char *fireSoundPath);

    ~Boss();

    // Call every frame from the game loop
    // playerX : player's current X position, used to measure distance & aim
    void update(float dt, float playerX);

    void draw() override;

    // Returns true once the boss has fired all its projectiles and vanished
    bool isDefeated() const;
    Rectangle getCoreBox() const; // needed by GameManager for the boss barrier

    // Lets GameManager iterate over live projectiles for collision tests
    const vector<Projectile> &getProjectiles() const;

private:
    Rectangle mapBorder;

    Image projectileImage;       // CPU-side image loaded in constructor
    Texture2D projectileTexture; // GPU texture, created on first update()
    bool textureHasLoaded;       // false until projectileTexture is uploaded
    vector<Projectile> projectiles;

    int maxShots;        // total shots allowed
    int shotsFired;      // how many have been fired so far
    float shootCooldown; // seconds between shots
    float shootTimer;    // counts down to next shot

    bool aggroed;  // true once player is within trigger distance
    bool defeated; // true once shotsFired == maxShots

    void spawnProjectile(float playerX);
    void advanceAnimation(Animation &anim, float dt);
};