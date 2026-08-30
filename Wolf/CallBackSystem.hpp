#pragma once
#include "System.hpp"
class GameScene;
class CallBackSystem : public System {
public:
    CallBackSystem(GameScene* scene);
    void update(World* world) override;

private:
    GameScene* scene;
};