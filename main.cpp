// main.cpp
#include "GameManager.hpp"

int main()
{
    // Game window settings
    const int screenWidth = 1280;
    const int screenHeight = 720;
    const char *windowTitle = "My Game";

    // Create the game manager (initializes window, audio, assets)
    GameManager game(screenWidth, screenHeight, windowTitle, 30,
                     "assets/win.mp3",
                     "assets/lose.mp3");

    while (game.isRunning())
    {

        // Update game logic (state machine, entities, etc.)
        game.update(GetFrameTime());

        // Draw everything (render texture -> screen)
        game.draw();
    }

    // GameManager destructor will clean up resources automatically
    return 0;
}