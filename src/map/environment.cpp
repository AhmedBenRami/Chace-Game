#include "environment.h"

Environment::Environment(const char *levelFilePath, const char *bgPath, const char *groundPath,
                         const char *blockPath, const char *coinPath,
                         const char *treePath, const char *teleporterPath,
                         const char *gatePath)
{
    textureHasLoaded = false;

    // Load all assets as CPU images — no GPU calls yet
    backgroundImage = LoadImage(bgPath);
    groundImage = LoadImage(groundPath);
    blocksImage = LoadImage(blockPath);
    coinsImage = LoadImage(coinPath);
    treesImage = LoadImage(treePath);
    teleportersImage = LoadImage(teleporterPath);
    gateImage = LoadImage(gatePath);

    // Zero-init the Texture2D structs so IsTextureValid() returns false safely
    background = {0};
    ground = {0};
    blocks.texture = {0};
    coins.texture = {0};
    trees.texture = {0};
    teleporters.texture = {0};
    gate.texture = {0};
    gate.active = false; // locked until boss is defeated

    // Use the block image width as tileSize (same as before)
    tileSize = blocksImage.width;

    parseLevel(levelFilePath, tileSize);
    placeTrees(tileSize);

    // Place the gate at the far right of the world, sitting on the ground
    float gw = (float)gateImage.width;
    float gh = (float)gateImage.height;
    gate.rect = {worldBounds.x + worldBounds.width - gw - 4.0f,
                 worldBounds.y + worldBounds.height - gh,
                 gw, gh};
}

Environment::~Environment()
{
    if (textureHasLoaded)
    {
        UnloadTexture(background);
        UnloadTexture(ground);
        UnloadTexture(blocks.texture);
        UnloadTexture(coins.texture);
        UnloadTexture(trees.texture);
        UnloadTexture(teleporters.texture);
        UnloadTexture(gate.texture);
    }
    else
    {
        UnloadImage(backgroundImage);
        UnloadImage(groundImage);
        UnloadImage(blocksImage);
        UnloadImage(coinsImage);
        UnloadImage(treesImage);
        UnloadImage(teleportersImage);
        UnloadImage(gateImage);
    }
}

void Environment::parseLevel(const string &filePath, int tileSize)
{
    ifstream file(filePath);
    if (!file.is_open())
    {
        TraceLog(LOG_ERROR, "Failed to open level file: %s", filePath.c_str());
        return;
    }

    vector<string> lines;
    string line;
    int maxWidth = 0, height = 0;
    while (getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        lines.push_back(line);
        maxWidth = max(maxWidth, (int)line.length());
        height++;
    }

    int maxCols = maxWidth; // from reading the level file
    int maxRows = height;   // number of lines
    // cout << "maxCol : "<< maxCols << ", " <<
    // worldBounds = {0, 0, (float)maxCols * tileSize, (float)maxRows * tileSize};
    worldBounds = {0, 0, (float)groundImage.width, (float)GetScreenHeight() - groundImage.height};

    for (int row = 0; row < height; ++row)
    {
        const string &lineStr = lines[row];
        for (int col = 0; col < maxWidth; ++col)
        {
            char ch = (col < (int)lineStr.length()) ? lineStr[col] : ' ';
            float x = (float)col * tileSize;
            float y = (float)row * tileSize;
            Rectangle rect = {x, y, (float)tileSize, (float)tileSize};

            switch (ch)
            {
            case '#':
                blocks.rects.push_back(rect);
                break;
            case 'C':
                coins.coinsRec.push_back(rect);
                break;
            case 'T':
                teleporters.rects.push_back(rect);
                break;
            default:
                break;
            }
        }
    }
}

void Environment::placeTrees(int tileSize)
{
    static bool seeded = false;
    if (!seeded)
    {
        srand((unsigned)time(nullptr));
        seeded = true;
    }

    for (const auto &block : blocks.rects)
    {
        if (rand() % 100 < 20 && block.y > worldBounds.height / 2)
        {
            float treeX = block.x + (block.width - tileSize) / 2;
            float treeY = block.y - tileSize;
            Rectangle treeRect = {treeX, treeY, (float)tileSize, (float)tileSize};
            bool overlap = false;
            for (const auto &t : trees.rects)
            {
                if (CheckCollisionRecs(treeRect, t))
                {
                    overlap = true;
                    break;
                }
            }
            if (!overlap)
            {
                trees.rects.push_back(treeRect);
            }
        }
    }
}

void Environment::update()
{
    if (!textureHasLoaded)
    {
        background = LoadTextureFromImage(backgroundImage);
        ground = LoadTextureFromImage(groundImage);
        blocks.texture = LoadTextureFromImage(blocksImage);
        coins.texture = LoadTextureFromImage(coinsImage);
        trees.texture = LoadTextureFromImage(treesImage);
        teleporters.texture = LoadTextureFromImage(teleportersImage);
        gate.texture = LoadTextureFromImage(gateImage);

        UnloadImage(backgroundImage);
        UnloadImage(groundImage);
        UnloadImage(blocksImage);
        UnloadImage(coinsImage);
        UnloadImage(treesImage);
        UnloadImage(teleportersImage);
        UnloadImage(gateImage);

        textureHasLoaded = true;
    }
}

void Environment::drawBg() const
{
    if (!textureHasLoaded)
        return;
    // Background
    DrawTexturePro(background,
                   {0, 0, (float)background.width, (float)background.height},
                   {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                   {0, 0}, 0, WHITE);
}
void Environment::draw() const
{
    if (!textureHasLoaded)
        return;
    // ground
    DrawTexture(ground, worldBounds.x, worldBounds.height, WHITE);

    // Blocks
    for (const auto &rect : blocks.rects)
    {
        DrawTexturePro(blocks.texture,
                       {0, 0, (float)blocks.texture.width, (float)blocks.texture.height},
                       rect, {0, 0}, 0, WHITE);
    }

    // Coins (only uncollected)
    for (const auto &coin : coins.coinsRec)
    {

        DrawTexturePro(coins.texture,
                       {0, 0, (float)coins.texture.width, (float)coins.texture.height},
                       coin, {0, 0}, 0, WHITE);
    }

    // Trees
    for (const auto &rect : trees.rects)
    {
        DrawTexturePro(trees.texture,
                       {0, 0, (float)trees.texture.width, (float)trees.texture.height},
                       rect, {0, 0}, 0, WHITE);
    }

    // Teleporters
    for (const auto &rect : teleporters.rects)
    {
        DrawTexturePro(teleporters.texture,
                       {0, 0, (float)teleporters.texture.width, (float)teleporters.texture.height},
                       rect, {0, 0}, 0, WHITE);
    }

    // Gate — always drawn; tinted green when active (unlocked), dark when locked
    Color gateTint = gate.active ? GREEN : DARKGRAY;
    DrawTexturePro(gate.texture,
                   gate.rect,
                   gate.rect, {0, 0}, 0, gateTint);
}

void Environment::collectCoin(const Rectangle &coin)
{
    auto it = std::find_if(coins.coinsRec.begin(), coins.coinsRec.end(),
                           [&coin](const Rectangle &rect)
                           {
                               return rect.x == coin.x && rect.y == coin.y &&
                                      rect.width == coin.width && rect.height == coin.height;
                           });

    if (it != coins.coinsRec.end())
    {
        coins.coinsRec.erase(it);
    }
}

void Environment::activateGate()
{
    gate.active = true;
}