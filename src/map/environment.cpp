#include "environment.h"

Environment::Environment(const char *levelFilePath, const char *bgPath, const char *groundPath,
                         const char *blockPath, const char *coinPath,
                         const char *treePath, const char *teleporterPath,
                         const char *gatePath)
{

    background = LoadTexture(bgPath);
    ground = LoadTexture(groundPath);
    blocks.texture = LoadTexture(blockPath);
    coins.texture = LoadTexture(coinPath);
    trees.texture = LoadTexture(treePath);
    teleporters.texture = LoadTexture(teleporterPath);
    gate.texture = LoadTexture(gatePath);
    gate.active = false; // locked until boss is defeated

    tileSize = blocks.texture.width;

    parseLevel(levelFilePath, tileSize);
    placeTrees(tileSize);

    // Place the gate at the far right of the world, sitting on the ground
    float gw = (float)gate.texture.width;
    float gh = (float)gate.texture.height;
    gate.rect = {worldBounds.x + worldBounds.width - gw - 4.0f,
                 worldBounds.y + worldBounds.height - gh,
                 gw, gh};
}

Environment::~Environment()
{
    UnloadTexture(background);
    UnloadTexture(blocks.texture);
    UnloadTexture(coins.texture);
    UnloadTexture(trees.texture);
    UnloadTexture(teleporters.texture);
    UnloadTexture(gate.texture);
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
    worldBounds = {0, 0, (float)ground.width, (float)GetScreenHeight() - ground.height};

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
}

void Environment::drawBg() const
{
    // Background
    DrawTexturePro(background,
                   {0, 0, (float)background.width, (float)background.height},
                   {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                   {0, 0}, 0, WHITE);
}
void Environment::draw() const
{
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