#pragma once
#include "AppWindow.hpp"
#include "Input.hpp"
#include "SoundManager.hpp"
#include "PlayerData.hpp"

#include "GameScene.hpp"
#include <memory>
#define DEV
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
    std::unique_ptr<MenuScene> menuScene;
    PlayerData playerData = {};
    AppScreen gameScreen;
    Input gameInput;
    SoundManager audio;
#ifdef DEV
    bool enableChangeLevel = true;
    int maxLevel = 2;
#else
    bool enableChangeLevel = false;
    int maxLevel = 1;
#endif // 0
    

};
