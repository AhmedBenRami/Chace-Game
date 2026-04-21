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

Entity::Entity(const char *imagePath, Vector2 position)
{
    isPlayer = false;
    currentAnimation = new Animation;
    currentAnimation->tileTexture = LoadTexture(imagePath);

    if (!IsTextureValid(currentAnimation->tileTexture))
    {
        cout << "Error loading image: " << imagePath << endl;
        exit(3);
    }

    coreBox = {position.x, position.y, (float)currentAnimation->tileTexture.height, (float)currentAnimation->tileTexture.height};

    InitAnimation(*currentAnimation, coreBox);
}

Entity::Entity(const char *dirPath, Vector2 position, bool isPlayer)
{
    this->isPlayer = isPlayer;
    idleAnimation.tileTexture = LoadTexture(TextFormat("%s/player_idle.png", dirPath));
    runningAnimation.tileTexture = LoadTexture(TextFormat("%s/player_run.png", dirPath));
    jumpingAnimation.tileTexture = LoadTexture(TextFormat("%s/player_jump.png", dirPath));

    if (!IsTextureValid(idleAnimation.tileTexture) || !IsTextureValid(runningAnimation.tileTexture) || !IsTextureValid(jumpingAnimation.tileTexture))
    {
        cout << "Error loading player sprites from: " << dirPath << endl;
        exit(4);
    }

    // Assume all frames are square based on height
    float size = (float)idleAnimation.tileTexture.height;
    coreBox = {position.x, position.y, size, size};

    InitAnimation(idleAnimation, coreBox);
    InitAnimation(runningAnimation, coreBox);
    InitAnimation(jumpingAnimation, coreBox);

    physicalState = IDLE;
    currentAnimation = &idleAnimation;
}

void Entity::update()
{

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
    DrawTexturePro(currentAnimation->tileTexture, currentAnimation->currentFrame, coreBox, {0, 0}, 0, WHITE);
}

Entity::~Entity()
{
    if (isPlayer)
    {
        UnloadTexture(idleAnimation.tileTexture);
        UnloadTexture(runningAnimation.tileTexture);
        UnloadTexture(jumpingAnimation.tileTexture);
    }
    else
    {
        UnloadTexture(currentAnimation->tileTexture);
        delete currentAnimation;
    }
}
