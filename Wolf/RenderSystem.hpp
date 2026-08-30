#pragma once
#include "System.hpp"

class GameScene;
class Entity;
class RenderSystem : public System {
public:
    RenderSystem(GameScene* scene) :
    scene(scene)
    {}
    void update(World* world) override;
private:
    void drawBackground();
    void renderPlayerStats();
    void renderWeapon();
    void drawWalls();
    void renderDotEntity(Entity* entity);
    void renderDotEntities(World* world);

    GameScene* scene;
};