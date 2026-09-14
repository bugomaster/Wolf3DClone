#pragma once
#include "MathWolf.hpp"

enum class Collectible
{
    AMMO,
    GOLDBOX,
    TROPHIE,
    MEAL,
    MEATBALLS,
    BLUEKEY,
    GOLDKEY,
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
enum class Decoration
{
    LAMP,
    TREE,
    FLAG,
    TABLE
};
struct DecorationData
{
    Vector2f position;
    Decoration type;
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

};


struct LevelData
{
    Vector2f playerStart;
    Vector2f cameraStart;

    Vector2i endGate;
    std::vector<Door> doors;
    std::vector<SecretWall> secretWalls;
    std::vector<LockGate> lockGates;
    std::vector<CollectibleData> collectibles;
    std::vector<KeyData> keys;
    std::vector<DecorationData> decorations;
    std::vector<std::vector<int>> map;
    bool loadLevelProperties(const std::string& path);
};


