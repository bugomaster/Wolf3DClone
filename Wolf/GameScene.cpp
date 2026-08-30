#include "pch.hpp"
#include "Input.hpp"
#include "SoundManager.hpp"
#include "AppWindow.hpp"
#include "Systems.hpp"

 
#include "GameScene.hpp"



GameScene::GameScene(AppScreen* window, Input* input,SoundManager* audio)
{
    this->window = window;
    this->input = input;
    this->audio = audio;
}

bool GameScene::initScene() {

    this->finished = false;

    world = new World();
    

    world->registerSystem<MapSystem>(this);
    world->registerSystem<CallBackSystem>(this);
    world->registerSystem<PlayerSystem>(this);
    world->registerSystem<EnemySystem>(this);
    world->registerSystem<MovementSystem>(this);
    world->registerSystem<RayCastingSystem>(this);
    world->registerSystem<AnimationSystem>(this);
    world->registerSystem<RenderSystem>(this);

    world->loadScene(this);
    return true;

}


void GameScene::update(){
    world->tick();
}
void GameScene::render() {}
void GameScene::quitScene(){


    delete world;
    world = nullptr;
}
