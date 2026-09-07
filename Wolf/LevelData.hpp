#pragma once
#include "MathWolf.hpp"

enum class Collectible
{
    AMMO,
    GOLDBOX,
    TROPHIE,
    MEAL,
    MEATBALLS,
    KEY,
};
struct CollectibleData
{
    Vector2f position;
    Collectible type;
};

struct KeyData
{
    Vector2f position;
    int keyID;
};


struct SecretWall
{
    Vector2i position;
    Vector2i dirMove;
    int tileID;

};
struct LockGate
{
    Vector2i position;
    int tileID;
    int keyID;

};

struct Door
{
    Vector2i position;
    Vector2i dirMove;
    int tileID;

};

struct LevelData
{
    Vector2f playerStart;
    Vector2f cameraStart;

    std::vector<Door> doors;
    std::vector<SecretWall> secretWalls;
    std::vector<LockGate> lockGates;
    std::vector<CollectibleData> collectibles;
    std::vector<KeyData> keys;

    bool loadLevelProperties(const std::string& path);
};


