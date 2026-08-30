#pragma once
#include "System.hpp"

class GameScene;
class Entity;

class MovementSystem : public System {
public:
    MovementSystem(GameScene* scene):
    scene(scene)
    {}
    void update(World* world) override;
private:

    GameScene* scene;
};
