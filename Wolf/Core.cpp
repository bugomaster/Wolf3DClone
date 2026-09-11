#include "Core.hpp"
#include <SDL_Image.h>
#include <iostream>
#include <memory>

#include "AssetsLoads.hpp"
#include "Map.hpp"
#include "GFX.hpp"



bool Core::init() {
    bool success = true;

    //
    SDL_Init(SDL_INIT_VIDEO);
    success &= gameScreen.init();
    g_assets.loadAll(gameScreen.renderer);

    audio.init();

    //
    menuScene = std::make_unique<MenuScene>(&gameScreen, &gameInput, &audio);
    menuScene.get()->initScene();
    gameScene = std::make_unique<GameScene>(&gameScreen, &gameInput, &audio);
    gameScene.get()->initScene();
    //menuScene.get()->firstRun = true;
    

    this->scene = gameScene.get();

    return success;
}
void Core::limitFPS(Uint64 startTick, SDL_Window* window) {
    Uint64 frameTime = SDL_GetTicks64() - startTick;
    Uint64 targetTime = 1000 / GFX::MAX_FPS;

    double fps = (frameTime > 0) ? (1000.0 / frameTime) : 0.0;
    std::string title = "FPS: " + std::to_string(static_cast<int>(fps));
    SDL_SetWindowTitle(window, title.c_str());
    if (frameTime < targetTime) {
        SDL_Delay(static_cast<Uint32>(targetTime - frameTime));
    }
}
void Core::run() {
    if (!init())
    {
        quit();
        return;
    }
    while (gameIsRunning) {
        Uint64 startTicks = SDL_GetTicks64();
        gameIsRunning = gameInput.update();
        //scene->handleInput();//--> for menuScene


        scene->update();//gameScene tick
        if (scene->isFinished())
        {
            auto* ptrMenuS = dynamic_cast<MenuScene*>(scene);
            auto* ptrGameS = dynamic_cast<GameScene*>(scene);
            if (ptrMenuS)
            {
                // start new game
                if (ptrMenuS->isNewGame()) {
                    if (gameScene.get())
                    {
                        gameScene.get()->quitScene();
                    }
                    gameScene = std::make_unique<GameScene>(&gameScreen, &gameInput, &audio);
                    gameScene.get()->initScene();

                }
                else {
                    gameScene.get()->setFinished(false);
                }


                this->scene = gameScene.get();

            }
            else if (ptrGameS)
            {
                menuScene = std::make_unique<MenuScene>(&gameScreen, &gameInput, &audio);
                menuScene.get()->initScene();
                menuScene.get()->setPlayerDead(ptrGameS->isPlayerDead());
                this->scene = menuScene.get();
            }
        }

        gameScreen.update();
        limitFPS(startTicks, scene->getScreen()->window);


    }

    quit();
}
void Core::quit() {
    if (menuScene)
        menuScene->quitScene();
    if (gameScene)
        gameScene->quitScene();
    g_assets.destroyAll();
    gameScreen.destroy();
    audio.shutdown();
    SDL_Quit();

}