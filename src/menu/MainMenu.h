#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <functional>

using namespace std;

struct Title
{
    const char *title;
    int fontSize;
    Vector2 size;
    Vector2 position;
    Color color;
};

struct MenuButton
{
    Rectangle rect;
    string label;
    Rectangle textRec;
    int fontSize;
    bool hovered;
    Color normalBg, normalBorder;
    Color hoverBg, hoverBorder;
    Color normalText, hoverText;
};

struct POPUP
{
    Rectangle border;
    Color borderColor, bg;
    vector<string> infos;
    vector<Vector2> positions;
    float fontSize;
    Color textColor;
};

class MainMenu
{
public:
    MainMenu(int screenW, int screenH, Font font, const char *musicPath = nullptr);
    ~MainMenu();

    void Update();
    void Draw();

    void stopMusic();
    void playMusic();

    // Callbacks set by GameManager
    function<void()> onStart;
    function<void()> onExit;

private:
    void DrawAboutPopup();
    void BuildTitle();
    void BuildButtons();
    void BuildPopup();

    int screenW_, screenH_;
    bool showAbout_ = false;

    // Title and buttons
    Title mainTitle_;
    Title subTitle_;
    vector<MenuButton> buttons_;
    POPUP aboutPopup_;

    float titleBob_ = 0.0f;
    float time_ = 0.0f;

    // Background
    Texture2D bgTexture_ = {0};
    Font font_;

    Sound menuSound_; // ambient / music played while the menu is open
};