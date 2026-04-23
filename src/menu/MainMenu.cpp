#include "MainMenu.h"
#include "raylib.h"
#include <cmath>

MainMenu::MainMenu(int screenW, int screenH, Font font, const char *musicPath)
    : screenW_(screenW), screenH_(screenH), font_(font)
{
    BuildTitle();
    BuildButtons();
    BuildPopup();

    if (FileExists("assets/menu_bg.png"))
        bgTexture_ = LoadTexture("assets/menu_bg.png");

    menuSound_ = {0};
    if (musicPath)
    {
        menuSound_ = LoadSound(musicPath);
    }
}

MainMenu::~MainMenu()
{
    if (bgTexture_.id > 0)
        UnloadTexture(bgTexture_);
    StopSound(menuSound_);
    UnloadSound(menuSound_);
}

void MainMenu::BuildTitle()
{
    mainTitle_.title = "PLATFORMER QUEST";
    mainTitle_.fontSize = 64;
    mainTitle_.size = MeasureTextEx(font_, mainTitle_.title, mainTitle_.fontSize, 1);
    mainTitle_.position.x = screenW_ / 2 - mainTitle_.size.x / 2;
    mainTitle_.color = {153, 0, 0, 255};

    subTitle_.title = "An Action Platformer Adventure";
    subTitle_.fontSize = 20;
    subTitle_.size = MeasureTextEx(font_, subTitle_.title, subTitle_.fontSize, 1);
    subTitle_.position.x = screenW_ / 2 - subTitle_.size.x / 2;
    subTitle_.color = {10, 10, 10, 255};
}

void MainMenu::BuildButtons()
{
    float cx = screenW_ / 2.0f;
    float top = screenH_ / 2.0f - 20;
    float bw = 220, bh = 52, gap = 70;

    Color normalBG = {80, 80, 80, 255};
    Color normalBorder = {30, 45, 45, 255};
    Color hoverBG = {30, 45, 45, 127};
    Color hoverBorder = {30, 45, 45, 255};

    Vector2 startSize = MeasureTextEx(font_, "START", 26, 1);
    Vector2 aboutSize = MeasureTextEx(font_, "ABOUT", 26, 1);
    Vector2 exitSize = MeasureTextEx(font_, "EXIT", 26, 1);

    // Start button
    MenuButton start;
    start.rect = {cx - bw / 2, top, bw, bh};
    start.label = "START";
    start.textRec = {(start.rect.x + start.rect.width / 2 - startSize.x / 2), (start.rect.y + start.rect.height / 2 - 13), startSize.x, startSize.y};
    start.fontSize = 26;
    start.hovered = false;
    start.normalBg = normalBG;
    start.normalBorder = normalBorder;
    start.hoverBg = hoverBG;
    start.hoverBorder = hoverBorder;
    start.normalText = {255, 255, 255, 255};
    start.hoverText = {180, 180, 180, 255};

    // About button
    MenuButton about;
    about.rect = {cx - bw / 2, top + gap, bw, bh};
    about.label = "ABOUT";
    about.textRec = {(about.rect.x + about.rect.width / 2 - aboutSize.x / 2), (about.rect.y + about.rect.height / 2 - 13), aboutSize.x, aboutSize.y};
    about.fontSize = 26;
    about.hovered = false;
    about.normalBg = normalBG;
    about.normalBorder = normalBorder;
    about.hoverBg = hoverBG;
    about.hoverBorder = hoverBorder;
    about.normalText = {255, 255, 255, 255};
    about.hoverText = {180, 180, 180, 255};

    // Exit button
    MenuButton exit;
    exit.rect = {cx - bw / 2, top + gap * 2, bw, bh};
    exit.label = "EXIT";
    exit.textRec = {(exit.rect.x + exit.rect.width / 2 - exitSize.x / 2), (exit.rect.y + exit.rect.height / 2 - 13), exitSize.x, exitSize.y};
    exit.fontSize = 26;
    exit.hovered = false;
    exit.normalBg = normalBG;
    exit.normalBorder = normalBorder;
    exit.hoverBg = hoverBG;
    exit.hoverBorder = hoverBorder;
    exit.normalText = {255, 255, 255, 255};
    exit.hoverText = {180, 180, 180, 255};

    buttons_ = {start, about, exit};
}

void MainMenu::BuildPopup()
{
    float pw = screenW_ - 400, ph = screenH_ - 300;
    aboutPopup_.border = {screenW_ / 2.0f - pw / 2, screenH_ / 2.0f - ph / 2, pw, ph};
    aboutPopup_.bg = {80, 80, 80, 255};
    aboutPopup_.borderColor = {30, 45, 45, 255};

    aboutPopup_.infos = {
        "A fully-featured 2D platformer built with Raylib.",
        "Navigate levels, defeat enemies, collect coins,",
        "and defeat the Boss to complete each stage.",
        "",
        "Arrow Keys  -  Move",
        "Space       -  Jump",
        "Enter       -  Pause / Resume",
        "Escape      -  Exit game",
        "",
        "Survive the timer, beat the boss, reach the gate",
        "to advance. 3 levels of increasing difficulty.",
        "",
        "Click anywhere to close this popup"};

    for (int i = 0; i < aboutPopup_.infos.size(); ++i)
    {
        aboutPopup_.positions.push_back({aboutPopup_.border.x + 20, aboutPopup_.border.y + 64 + i * 22}); //, 16, 1, {200, 200, 230, 255});
    }

    aboutPopup_.fontSize = 16;
    aboutPopup_.textColor = {200, 200, 230, 255};
}
void MainMenu::Update()
{
    time_ += GetFrameTime();
    titleBob_ = sinf(time_ * 2.0f) * 8.0f;

    // Keep menu music looping
    if (menuSound_.frameCount > 0 && !IsSoundPlaying(menuSound_))
        PlaySound(menuSound_);

    Vector2 mouse = GetMousePosition();

    for (int i = 0; i < (int)buttons_.size(); ++i)
    {
        buttons_[i].hovered = CheckCollisionPointRec(mouse, buttons_[i].rect);
    }

    if (showAbout_)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            showAbout_ = false;
        }
        // Close on ESC
        if (IsKeyPressed(KEY_ESCAPE))
            showAbout_ = false;
        return;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (buttons_[0].hovered && onStart)
            onStart();
        if (buttons_[1].hovered)
            showAbout_ = true;
        if (buttons_[2].hovered && onExit)
            onExit();
    }
}

void MainMenu::Draw()
{
    // Static background image (falls back to plain colour if missing)
    if (bgTexture_.id > 0)
    {
        DrawTexturePro(
            bgTexture_,
            {0, 0, (float)bgTexture_.width, (float)bgTexture_.height},
            {0, 0, (float)screenW_, (float)screenH_},
            {0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawRectangleGradientV(0, 0, screenW_, screenH_,
                               {10, 10, 40, 255}, {30, 10, 80, 255});
    }

    // Title
    mainTitle_.position.y = (int)(screenH_ / 2 - 160 + titleBob_);

    // Shadow
    DrawTextEx(font_, mainTitle_.title, {mainTitle_.position.x + 4, mainTitle_.position.y + 4}, mainTitle_.fontSize, 1, {0, 0, 0, 120});
    // Gradient simulation (two halves)
    DrawTextEx(font_, mainTitle_.title, mainTitle_.position, mainTitle_.fontSize, 1, mainTitle_.color);

    // Subtitle
    subTitle_.position.y = mainTitle_.position.y + mainTitle_.fontSize + 6;
    DrawTextEx(font_, subTitle_.title, subTitle_.position, subTitle_.fontSize, 1, subTitle_.color);

    // Buttons
    for (auto &btn : buttons_)
    {
        Color bg = btn.hovered ? btn.hoverBg : btn.normalBg;
        Color border = btn.hovered ? btn.hoverBorder : btn.normalBorder;

        DrawRectangleRec(btn.rect, bg);
        DrawRectangleLinesEx(btn.rect, 2.0f, border);

        Color textCol = btn.hovered ? btn.hoverText : btn.normalText;
        DrawTextEx(font_, btn.label.c_str(), {btn.textRec.x, btn.textRec.y}, btn.fontSize, 1, textCol);
    }

    // Version
    DrawText("v1.0", 8, screenH_ - 22, 14, {80, 80, 120, 180});

    if (showAbout_)
        DrawAboutPopup();
}

void MainMenu::DrawAboutPopup()
{
    // Dim overlay
    DrawRectangle(0, 0, screenW_, screenH_, {0, 0, 0, 160});

    // Panel
    DrawRectangleRec(aboutPopup_.border, aboutPopup_.bg);
    DrawRectangleLinesEx(aboutPopup_.border, 2.0f, aboutPopup_.borderColor);

    // text
    for (int i = 0; i < aboutPopup_.infos.size(); i++)
    {
        string &info = aboutPopup_.infos.at(i);
        DrawTextEx(font_, info.c_str(), aboutPopup_.positions.at(i), aboutPopup_.fontSize, 1, aboutPopup_.textColor);
    }
}

void MainMenu::stopMusic()
{
    if (IsSoundPlaying(menuSound_))
        StopSound(menuSound_);
}

void MainMenu::playMusic()
{
    if (!IsSoundPlaying(menuSound_))
        PlaySound(menuSound_);
}
