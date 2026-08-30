#pragma once
#include "System.hpp"

class Entity;
struct MoveData;
class GameScene;
class PlayerSystem : public System
{
public:

    PlayerSystem(GameScene* gameScene);
    void onAddedToWorld(World* world) override;
    void update(World* world) override;

private:
    void initPlayer();
    void updateCamera();
    void updateInput();
    void updateDoorOpen();
    void updateShooting();
    void updateMovement();
    void updateState();
    void checkCollectibleCollision(World* world, MoveData& moveData);
    void updatePos(World* world);

   



private:
    GameScene* gameScene;
    Entity* playerEntity = nullptr;


};


