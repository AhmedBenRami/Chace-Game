#include "entity.h"

using namespace std;

// Helper to initialize animation values to avoid repetition
void InitAnimation(Animation &anim, Rectangle coreBox)
{
    anim.currentFrame = {0, 0, coreBox.width, coreBox.height};
    anim.firstIndex = 0;
    anim.currentIndex = 0;
    anim.lastIndex = (int)(anim.tileTexture.width / anim.tileTexture.height);
    anim.speed = 0.15f;
    anim.timeLeft = 0.15f;
}

// ── Non-player constructor — load image only, defer texture upload ──────────
Entity::Entity(const char *imagePath, Vector2 position)
{
    isPlayer = false;
    textureHasLoaded = false;
    currentAnimation = new Animation;
    entitySound = {0};  // zero-init; subclass constructors load the actual sound

    singleImage = LoadImage(imagePath);
    if (singleImage.data == nullptr)
    {
        cout << "Error loading image: " << imagePath << endl;
        exit(3);
    }

    // Use image dimensions to size the coreBox before any texture exists
    float size = (float)singleImage.height;
    coreBox = {position.x, position.y, size, size};

    // Partially init animation (texture not valid yet — filled in update())
    currentAnimation->currentFrame = {0, 0, size, size};
    currentAnimation->firstIndex = 0;
    currentAnimation->currentIndex = 0;
    currentAnimation->lastIndex = singleImage.width / singleImage.height;
    currentAnimation->speed = 0.15f;
    currentAnimation->timeLeft = 0.15f;
}

// ── Player constructor — load images only, defer texture upload ─────────────
Entity::Entity(const char *dirPath, Vector2 position, bool isPlayer)
{
    this->isPlayer = isPlayer;
    textureHasLoaded = false;
    entitySound = {0};  // zero-init; Player constructor loads the actual sound

    idleImage    = LoadImage(TextFormat("%s/player_idle.png",  dirPath));
    runningImage = LoadImage(TextFormat("%s/player_run.png",   dirPath));
    jumpingImage = LoadImage(TextFormat("%s/player_jump.png",  dirPath));

    if (idleImage.data == nullptr || runningImage.data == nullptr || jumpingImage.data == nullptr)
    {
        cout << "Error loading player images from: " << dirPath << endl;
        exit(4);
    }

    float size = (float)idleImage.height;
    coreBox = {position.x, position.y, size, size};

    // Init all three animations from image dimensions (no GPU texture yet)
    auto initFromImage = [](Animation &anim, const Image &img, Rectangle box)
    {
        anim.currentFrame = {0, 0, box.width, box.height};
        anim.firstIndex   = 0;
        anim.currentIndex = 0;
        anim.lastIndex    = img.width / img.height;
        anim.speed        = 0.15f;
        anim.timeLeft     = 0.15f;
    };

    initFromImage(idleAnimation,    idleImage,    coreBox);
    initFromImage(runningAnimation, runningImage, coreBox);
    initFromImage(jumpingAnimation, jumpingImage, coreBox);

    physicalState    = IDLE;
    currentAnimation = &idleAnimation;
}

// ── Lazy texture upload — called once on the first update() ─────────────────
void Entity::update()
{
    if (!textureHasLoaded)
    {
        if (isPlayer)
        {
            idleAnimation.tileTexture    = LoadTextureFromImage(idleImage);
            runningAnimation.tileTexture = LoadTextureFromImage(runningImage);
            jumpingAnimation.tileTexture = LoadTextureFromImage(jumpingImage);

            UnloadImage(idleImage);
            UnloadImage(runningImage);
            UnloadImage(jumpingImage);
        }
        else
        {
            currentAnimation->tileTexture = LoadTextureFromImage(singleImage);
            UnloadImage(singleImage);
        }
        textureHasLoaded = true;
    }

    currentAnimation->timeLeft -= GetFrameTime();
    if (currentAnimation->timeLeft <= 0)
    {
        currentAnimation->currentIndex = (currentAnimation->currentIndex >= currentAnimation->lastIndex - 1) ? 0 : currentAnimation->currentIndex + 1;
        currentAnimation->currentFrame.x = (float)currentAnimation->currentIndex * currentAnimation->currentFrame.width;
        currentAnimation->timeLeft = currentAnimation->speed;
    }
}

void Entity::playerUpdate()
{
    if (!textureHasLoaded)
    {
        idleAnimation.tileTexture    = LoadTextureFromImage(idleImage);
        runningAnimation.tileTexture = LoadTextureFromImage(runningImage);
        jumpingAnimation.tileTexture = LoadTextureFromImage(jumpingImage);

        UnloadImage(idleImage);
        UnloadImage(runningImage);
        UnloadImage(jumpingImage);

        textureHasLoaded = true;
    }

    switch (physicalState)
    {
    case IDLE:
        currentAnimation = &idleAnimation;
        break;
    case RUNNING:
        currentAnimation = &runningAnimation;
        break;
    case JUMPING:
        currentAnimation = &jumpingAnimation;
        break;
    }

    currentAnimation->timeLeft -= GetFrameTime();
    if (currentAnimation->timeLeft <= 0)
    {
        currentAnimation->currentIndex = (currentAnimation->currentIndex >= currentAnimation->lastIndex - 1) ? 0 : currentAnimation->currentIndex + 1;
        currentAnimation->currentFrame.x = (float)currentAnimation->currentIndex * currentAnimation->currentFrame.width;
        currentAnimation->timeLeft = currentAnimation->speed;
    }
}

void Entity::draw()
{
    if (!textureHasLoaded) return; // nothing to draw yet
    DrawTexturePro(currentAnimation->tileTexture, currentAnimation->currentFrame, coreBox, {0, 0}, 0, WHITE);
}

Entity::~Entity()
{
    UnloadSound(entitySound);

    if (isPlayer)
    {
        if (textureHasLoaded)
        {
            UnloadTexture(idleAnimation.tileTexture);
            UnloadTexture(runningAnimation.tileTexture);
            UnloadTexture(jumpingAnimation.tileTexture);
        }
        else
        {
            // Textures were never uploaded — unload the raw images instead
            UnloadImage(idleImage);
            UnloadImage(runningImage);
            UnloadImage(jumpingImage);
        }
    }
    else
    {
        if (textureHasLoaded)
            UnloadTexture(currentAnimation->tileTexture);
        else
            UnloadImage(singleImage);

        delete currentAnimation;
    }
}

