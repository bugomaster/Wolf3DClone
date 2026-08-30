#pragma once
#include "System.hpp"
#include "GFX.hpp"


class GameScene;
class Entity;
class MapSystem : public System {
public:
    MapSystem(GameScene* gameScene);

    void update(World* world) override;
    void onAddedToWorld(World* world) override;
    static void createCollectibleEntity(World* world, Vector2f worldPos, int collectibleID);
    void createDoorHor(World* world, Vector2f posDoor);
    void createDoorVer(World* world, Vector2f posDoor);
    static Entity* gridObjectsMap[GFX::MAP_H][GFX::MAP_W];
private:
    GameScene* gameScene;

};