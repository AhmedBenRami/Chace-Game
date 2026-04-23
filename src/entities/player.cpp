// player.cpp
#include "player.h"
#include <cmath>

Player::Player(const char *dirPath, Vector2 startPos, Rectangle worldBounds, Font font,
               const char *runSoundPath, const char *jumpSoundPath, const char *damageSoundPath, const char *coinCollectSoundPath, int HP)
    : Entity(dirPath, startPos, true),
      mapBorder(worldBounds), originalPosition(startPos), hudFont(font)
{
    gravity = 1800.0f;
    jumpForce = -700.0f;
    moveSpeed = 300.0f;
    velocity = {0.0f, 0.0f};
    onGround = false;
    facingRight = true;
    health = HP;
    originalHealth = health;
    coinsCollected = 0;
    damageCooldown = 2.0f;
    damageTimer = 0.0f;
    finishedLevel = false;
    reachedGate = false;

    // Hologram rotation tracking
    holoRotation = 0.0f;
    holoRotationProgress = 0.0f;
    holoCycleComplete = false;

    jumpSound = {0};
    damageSound = {0};
    coinCollectSound = {0};

    // Load player sounds — entitySound (inherited) = looped running footstep
    if (runSoundPath)
        entitySound = LoadSound(runSoundPath);
    if (jumpSoundPath)
        jumpSound = LoadSound(jumpSoundPath);
    if (damageSoundPath)
        damageSound = LoadSound(damageSoundPath);
    if (coinCollectSoundPath)
        coinCollectSound = LoadSound(coinCollectSoundPath);

    float groundY = mapBorder.y + mapBorder.height - coreBox.height;
    coreBox.y = groundY;

    // collisionBox: half the sprite width, same height, centred on the sprite
    float cbW = coreBox.width * 0.5f;
    collisionBox = {coreBox.x + (coreBox.width - cbW) / 2.0f,
                    coreBox.y,
                    cbW, coreBox.height};
}

void Player::update(float dt)
{
    if (damageTimer > 0.0f)
        damageTimer -= dt;

    // Update hologram rotation - ease-in-out over 5 rotations (360° * 5 = 1800°)
    // Each complete cycle (0 to 1) represents one ease-in-out curve
    const float CYCLE_DURATION = 3.0f; // 3 seconds for 5 full rotations
    const float TOTAL_ROTATIONS = 5.0f;
    const float MAX_ANGLE = 360.0f * TOTAL_ROTATIONS; // 1800 degrees

    holoRotationProgress += dt / CYCLE_DURATION;

    if (holoRotationProgress >= 1.0f)
    {
        holoRotationProgress = 0.0f;
        holoCycleComplete = true;
    }

    // Ease-in-out cubic: smooth start, fast middle, smooth end
    // t = progress (0 to 1)
    float t = holoRotationProgress;
    // Cubic ease-in-out: if t < 0.5: 4t³, else: 1 - (-2t + 2)³ / 2
    float easeT;
    if (t < 0.5f)
    {
        easeT = 4.0f * t * t * t;
    }
    else
    {
        easeT = 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }

    // Calculate current rotation angle: goes from 0 to MAX_ANGLE following the ease curve
    holoRotation = easeT * MAX_ANGLE;

    // 1. Input (horizontal + jump) — BEFORE resetting onGround so jump() sees the correct state
    handleInput();

    // Save last frame's onGround BEFORE resetting — collideWithBlock() from the game loop
    // runs AFTER update() returns, so onGround won't be re-set in time for the animation check below.
    // Using wasOnGround means: "was I on the ground at the end of last frame?" which is one frame
    // behind but visually correct and avoids the stuck-in-jump-animation bug.
    bool wasOnGround = onGround;

    // *** Reset onGround — block/floor code will re-set it if needed this frame ***
    onGround = false;

    // 2. Gravity (always accumulate — enforceWorldBounds/collideWithBlock will stop it)
    velocity.y += gravity * dt;

    // 3. Move — both boxes move together so enforceWorldBounds sees correct positions
    coreBox.x += velocity.x * dt;
    coreBox.y += velocity.y * dt;
    collisionBox.x = coreBox.x + (coreBox.width - collisionBox.width) / 2.0f;
    collisionBox.y = coreBox.y;

    // 4. World boundary clamp
    enforceWorldBounds();

    // 5. Animation state — use wasOnGround (last frame) so block collisions are included
    if (wasOnGround)
        physicalState = (velocity.x == 0.0f) ? IDLE : RUNNING;
    else
        physicalState = JUMPING;

    // ── Running sound: loop while running on the ground, stop otherwise ──────
    if (physicalState == RUNNING)
    {
        if (!IsSoundPlaying(entitySound))
            PlaySound(entitySound);
    }
    else
    {
        if (IsSoundPlaying(entitySound))
            StopSound(entitySound);
    }

    // 6. Advance animation frame
    Entity::playerUpdate();
}

void Player::jump()
{
    if (onGround)
    {
        velocity.y = jumpForce;
        onGround = false;
        physicalState = JUMPING;
        PlaySound(jumpSound); // single-shot on leave ground
    }
}

void Player::handleInput()
{
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    {
        velocity.x = -moveSpeed;
        facingRight = false;
    }
    else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    {
        velocity.x = moveSpeed;
        facingRight = true;
    }
    else
        velocity.x = 0.0f;

    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        jump();
}

void Player::draw()
{
    // Sprite is drawn at the full coreBox so it looks right visually
    Rectangle srcRect = currentAnimation->currentFrame;
    if (!facingRight)
        srcRect.width = -srcRect.width;

    Color tint = WHITE;
    if (damageTimer > 0.0f)
        tint = RED;

    // Holo aura — centered on the player, rotating with ease-in-out style
    Rectangle holoDest = {
        coreBox.x + collisionBox.width / 2.0f,
        coreBox.y + coreBox.height / 2.0f,
        coreBox.width,
        coreBox.height};

    // Alternative: scale the hologram slightly larger than the player
    // holoDest = coreBox; // Use this line instead of above if you want exact same size

    DrawTexturePro(holoTexture,
                   {0, 0, (float)holoTexture.width, (float)holoTexture.height},
                   holoDest,
                   {holoDest.width / 2.0f, holoDest.height / 2.0f}, // pivot at center
                   holoRotation,                                    // rotation angle in degrees
                   WHITE);

    DrawTexturePro(currentAnimation->tileTexture, srcRect, coreBox, {0, 0}, 0, tint);
}

void Player::collideWithBlock(Rectangle block)
{
    float overlapLeft = (collisionBox.x + collisionBox.width) - block.x;
    float overlapRight = (block.x + block.width) - collisionBox.x;
    float overlapTop = (collisionBox.y + collisionBox.height) - block.y;
    float overlapBottom = (block.y + block.height) - collisionBox.y;

    if (overlapLeft <= 0 || overlapRight <= 0 || overlapTop <= 0 || overlapBottom <= 0)
        return;

    float minX = min(overlapLeft, overlapRight);
    float minY = min(overlapTop, overlapBottom);

    float offset = (coreBox.width - collisionBox.width) / 2.0f; // keeps sprite centred

    if (minY <= minX)
    {
        if (overlapTop < overlapBottom)
        {
            collisionBox.y = block.y - collisionBox.height;
            velocity.y = 0.0f;
            onGround = true;
        }
        else
        {
            collisionBox.y = block.y + block.height;
            velocity.y = 0.0f;
        }
        coreBox.y = collisionBox.y; // height is shared
    }
    else
    {
        if (overlapLeft < overlapRight)
        {
            collisionBox.x = block.x - collisionBox.width;
            velocity.x = 0.0f;
        }
        else
        {
            collisionBox.x = block.x + block.width;
            velocity.x = 0.0f;
        }
        coreBox.x = collisionBox.x - offset; // re-centre sprite
    }
}

void Player::collideWithTeleporter()
{
    coreBox.x = originalPosition.x;
    coreBox.y = originalPosition.y;
    collisionBox.x = coreBox.x + (coreBox.width - collisionBox.width) / 2.0f;
    collisionBox.y = coreBox.y;
    velocity = {0.0f, 0.0f};
    onGround = false;
}

Rectangle Player::getCoreBox() const { return collisionBox; }
Rectangle Player::getFullBox() const { return coreBox; }

void Player::enforceWorldBounds()
{
    float offset = (coreBox.width - collisionBox.width) / 2.0f;

    if (collisionBox.x < mapBorder.x)
    {
        collisionBox.x = mapBorder.x;
        coreBox.x = collisionBox.x - offset;
        velocity.x = 0.0f;
    }
    if (collisionBox.x + collisionBox.width > mapBorder.x + mapBorder.width)
    {
        collisionBox.x = mapBorder.x + mapBorder.width - collisionBox.width;
        coreBox.x = collisionBox.x - offset;
        velocity.x = 0.0f;
    }
    if (collisionBox.y < mapBorder.y)
    {
        collisionBox.y = mapBorder.y;
        coreBox.y = collisionBox.y;
        velocity.y = 0.0f;
    }

    float groundY = mapBorder.y + mapBorder.height - collisionBox.height;
    if (collisionBox.y >= groundY)
    {
        collisionBox.y = groundY;
        coreBox.y = collisionBox.y;
        velocity.y = 0.0f;
        onGround = true;
    }
}

void Player::collideWithEnemy()
{
    if (health <= 0)
        return;

    if (damageTimer <= 0.0f)
    {
        --health;
        damageTimer = damageCooldown; // start invincibility period
        PlaySound(damageSound);       // single-shot on damage
    }
}

void Player::collideWithCoin()
{
    ++coinsCollected;
    PlaySound(coinCollectSound);
}

void Player::collideWithProjectile()
{
    // Identical cooldown logic to collideWithEnemy
    collideWithEnemy();
}

bool Player::isFinishedLevel() const { return finishedLevel; }
bool Player::isDead() const { return health <= 0; }
bool Player::hasReachedGate() const { return reachedGate; }
void Player::setFinishedLevel(bool v) { finishedLevel = v; }
void Player::resetReachedGate() { reachedGate = false; }

Player::~Player()
{
    // entitySound (running) is unloaded by ~Entity()
    UnloadSound(jumpSound);
    UnloadSound(damageSound);
    UnloadSound(coinCollectSound);
}

void Player::collideWithGate()
{
    reachedGate = true;
}

void Player::drawPlayerInfos()
{
    float fontSize = 20.0f;
    float spacing = 1.0f;

    // Coins
    DrawTextEx(hudFont, TextFormat("Coins: %d", coinsCollected),
               {20, 20}, fontSize, spacing, WHITE);

    // Health as rectangles — one per HP, filled = alive, empty = lost
    DrawTextEx(hudFont, "HP:", {20, 52}, fontSize, spacing, WHITE);
    const float rectW = 22.0f, rectH = 22.0f, gap = 6.0f;
    float startX = 60.0f;
    for (int i = 0; i < originalHealth; ++i)
    {
        float x = startX + i * (rectW + gap);
        if (i < health)
            DrawRectangle((int)x, 52, (int)rectW, (int)rectH, RED);
        else
            DrawRectangle((int)x, 52, (int)rectW, (int)rectH, DARKGRAY);
        DrawRectangleLinesEx({x, 52, rectW, rectH}, 1, WHITE);
    }
}

void Player::stopSound()
{
    if (IsSoundPlaying(entitySound))
        StopSound(entitySound);
}

void Player::playSound()
{
    if (!IsSoundPlaying(entitySound))
        PlaySound(entitySound);
}