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
    static Entity* createCollectibleEntity(World* world, Vector2f worldPos, Collectible type, bool immidiate = false);
    static EntityID gridObjectsMap[GFX::MAP_H][GFX::MAP_W];
private:
    void createDoorHor(World* world, Vector2f position, int tileID, int sidesID, int timer);
    void createDoorVer(World* world, Vector2f position, int tileID, int sidesID, int timer);
    void createSecretMoveableWall(World* world, const SecretWall& wall);
    void createLockGate(World* world, const LockGate& lockGate, int face);
    GameScene* gameScene;

};