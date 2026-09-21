#include "Core.hpp"
#include <SDL_Image.h>
#include <iostream>
#include <memory>

#include "AssetsLoads.hpp"
#include "Map.hpp"
#include "GFX.hpp"
#include "Components.hpp"
bool Core::init() {
    bool success = true;

    //
    SDL_Init(SDL_INIT_VIDEO);
    success &= gameScreen.init();
    g_assets.loadAll(gameScreen.renderer);

    audio.init();

    //

    menuScene = std::make_unique<MenuScene>(&gameScreen, &gameInput, &audio, this->maxLevel, this->enableChangeLevel, true);
    menuScene.get()->initScene();

    gameScene = std::make_unique<GameScene>(&gameScreen, &gameInput, &audio);

#ifdef NO_MENU
    gameScene.get()->initScene();
    gameScene.get()->level = 1;
    this->scene = gameScene.get();
#else
    this->scene = menuScene.get();
#endif // NO_MENU



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
                if (ptrMenuS->newGame) {
                    if (gameScene.get())
                    {
                        gameScene.get()->quitScene();
                    }
                    gameScene.get()->level = menuScene.get()->chosenLevel;
                    gameScene.get()->initScene();
                    

                }
                else 
                {
                    gameScene.get()->setFinished(false);
                }


                this->scene = gameScene.get();

            }
            else if (ptrGameS)
            {
                if (ptrGameS->finishedLevel)// made it to the finish
                {
                    // game progress
                    if (this->maxLevel == ptrGameS->level)
                    {
                        if (this->maxLevel == GFX::MAX_LEVEL)
                        {
                            this->enableChangeLevel = true;
                            menuScene = std::make_unique<MenuScene>(&gameScreen, &gameInput, &audio, this->maxLevel, this->enableChangeLevel);
                            menuScene.get()->initScene();
                            this->scene = menuScene.get();
                        }
                        else
                        {
                            this->maxLevel++;
                            this->playerData = gameScene.get()->playerEntity->getComponent<PlayerComponent>()->data;
                            int nextLevel = gameScene.get()->level + 1;


                            gameScene.get()->quitScene();


                            //todo: cut Scene and then this
                            gameScene.get()->level = nextLevel;
                            gameScene.get()->initScene();
                            gameScene.get()->playerEntity->addComponent<PlayerComponent>(this->playerData);

                        }
                        
                    }
                    else// chose level
                    {
                        this->enableChangeLevel = true;
                        menuScene = std::make_unique<MenuScene>(&gameScreen, &gameInput, &audio, this->maxLevel, this->enableChangeLevel);
                        menuScene.get()->initScene();
                        menuScene.get()->canChooseLevel = true;
                        this->scene = menuScene.get();

                    }
                }
                else
                {//change scene to menu if player died of pressed esc
                    menuScene = std::make_unique<MenuScene>(&gameScreen, &gameInput, &audio, this->maxLevel, this->enableChangeLevel);
                    menuScene.get()->initScene();
                    menuScene.get()->deadPlayer = (ptrGameS->isPlayerDead());
                    this->scene = menuScene.get();
                }
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
        gameScene.get()->quitScene();
    g_assets.destroyAll();
    gameScreen.destroy();
    audio.shutdown();
    SDL_Quit();

}