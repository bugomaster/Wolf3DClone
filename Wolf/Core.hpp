#pragma once
#include "AppWindow.hpp"
#include "Input.hpp"
#include "SoundManager.hpp"

#include "GameScene.hpp"
#include <memory>

struct SDL_Window;
class Scene;


class Core {
public:
    bool init();
    void limitFPS(Uint64 startTick, SDL_Window* window);



    void run();

    void quit();

private:
    bool  gameIsRunning = true;
    Scene* scene = nullptr; 
    std::unique_ptr<GameScene> gameScene;

    AppScreen gameScreen;
    Input gameInput;
    SoundManager audio;
};
