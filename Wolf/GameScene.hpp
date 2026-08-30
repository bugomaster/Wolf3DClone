#pragma once
#include "Scene.hpp"
class World;
class Camera;
class Map;
class Entity;



class GameScene : public Scene {
public:
    GameScene(AppScreen* window, Input* input, SoundManager* audio);
    World* world;
    Entity* playerEntity;
    float yScreenOffset = 0.f;

public:
    void update() override;
    void handleInput() override{}
    void render() override;
    bool initScene() override;
    void quitScene() override;


private:
};
