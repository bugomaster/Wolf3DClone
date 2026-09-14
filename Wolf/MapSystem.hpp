#pragma once
#include "System.hpp"
#include "GFX.hpp"
#include "LevelData.hpp"


class GameScene;
class Entity;
class MapSystem : public System {
public:
    MapSystem(GameScene* gameScene);

    void update(World* world) override;
    void onAddedToWorld(World* world) override;
    static Entity* createCollectibleEntity(World* world, Vector2f worldPos, Collectible type);
    void createDoorHor(World* world, const Door& door);
    void createDoorVer(World* world, const Door& door);
    void createSecretMoveableWall(World* world, const SecretWall& wall);
    void createLockGate(World* world, const LockGate& lockGate);
    static EntityID gridObjectsMap[GFX::MAP_H][GFX::MAP_W];
private:
    GameScene* gameScene;

};