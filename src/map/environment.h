#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
using namespace std;

struct Block
{
    Texture2D texture;
    vector<Rectangle> rects;
};

struct Coin
{
    Texture2D texture;

    // const char *ground;
    vector<Rectangle> coinsRec;
};

struct Tree
{
    Texture2D texture;
    vector<Rectangle> rects;
};

struct Teleporter
{
    Texture2D texture;
    vector<Rectangle> rects;
};

struct Gate
{
    Texture2D texture;
    Rectangle rect; // single gate placed at the right edge of the level
    bool active;    // false = blocked (boss alive), true = player can pass
};

class Environment
{
public:
    Environment(const char *levelFilePath, const char *bgPath, const char *groundPath,
                const char *blockPath, const char *coinPath,
                const char *treePath, const char *teleporterPath,
                const char *gatePath);
    ~Environment();

    void update();
    void draw() const;
    void drawBg() const;
    Rectangle getWorldBounds() const { return worldBounds; }
    const Block &getBlocks() const { return blocks; }
    const Coin &getCoins() const { return coins; }
    const Teleporter &getTeleporters() const { return teleporters; }
    const Gate &getGate() const { return gate; }
    void collectCoin(const Rectangle &coin);
    void activateGate(); // call when boss is defeated

private:
    void parseLevel(const string &filePath, int tileSize);
    void placeTrees(int tileSize);

    bool textureHasLoaded; // false until GPU textures are uploaded from images

    // CPU-side images (loaded in constructor, freed after texture upload)
    Image backgroundImage;
    Image groundImage;
    Image blocksImage;
    Image coinsImage;
    Image treesImage;
    Image teleportersImage;
    Image gateImage;

    Texture2D background;
    Texture2D ground;
    Block blocks;
    Coin coins;
    Tree trees;
    Teleporter teleporters;
    Gate gate;
    Rectangle worldBounds;
    int tileSize;
};