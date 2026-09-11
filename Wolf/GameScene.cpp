#include "pch.hpp"
#include "Input.hpp"
#include "SoundManager.hpp"
#include "AppWindow.hpp"
#include "Systems.hpp"
#include "AssetsLoads.hpp"

 
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
    levelData.loadLevelProperties("Levels/Level_1/level_1.levelproperties");

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



BaseMenu::BaseMenu(AppScreen* window, Input* input, SoundManager* audio):
    cursorAnim(std::vector<int>{0,1}, 2, 32)
{

    this->window = window;
    this->input = input;
    this->audio = audio;
}
bool BaseMenu::initScene() {
    this->finished = false;
    return true;
}
void BaseMenu::quitScene() {

}

void BaseMenu::handleInput() 
{
    if (input->pressed(SDL_SCANCODE_RETURN))
    {
        this->finished = true;
        if (cursorIndex == 3)
            cursorIndex = -1;
        return;
    }
    else if (input->pressed(SDL_SCANCODE_ESCAPE))
    {
        this->finished = true;
        cursorIndex = -1;
        return;
    }
    else if (input->pressed(SDL_SCANCODE_UP))
    {
        cursorIndex -= 1;
    }
    else if (input->pressed(SDL_SCANCODE_DOWN))
    {
        cursorIndex += 1;
    }
    cursorIndex = std::max(cursorIndex, 0);
    cursorIndex = std::min(cursorIndex, 5);
}
void BaseMenu::update()
{
    //tick
    cursorAnim.tick();
}
void BaseMenu::render()
{

    //render cursor 

    int y_padd = 100;
    this->getScreen()->drawRect(SDL_Rect{ 0,0 ,GFX::SCREEN_WIDTH ,GFX::SCREEN_HEIGHT }, menuBGColor);
    this->getScreen()->drawRect(SDL_Rect{ 150,50 + y_padd,500 ,500 }, otherBGColor);
    this->getScreen()->renderText(410, 80 + y_padd, 70, "New Game", COLORS::WHITE);
    //this->getScreen()->renderText(0, 20 ,20, "Sound", COLORS::WHITE);
    this->getScreen()->renderText(410, 150 + y_padd, 70, "Control", COLORS::WHITE);
    this->getScreen()->renderText(410, 220 + y_padd, 70, "Load Game", COLORS::WHITE);
    this->getScreen()->renderText(410, 290 + y_padd, 70, "Quit", COLORS::WHITE);
    this->getScreen()->blitPixelsFromTextureScale(g_assets.menuScene.optionPointer.texture,
        SDL_Rect{ 25 * (cursorAnim.getFrameID()),0,25 ,16 }, 
        SDL_Rect{ 190, this->cursorIndex * 70 + 60 + y_padd,(int)(70.f * 1.7f) ,70});

    this->getScreen()->blitTextureScale(g_assets.menuScene.optionsLogo.texture,
        SDL_Rect{ 160,0,400 ,0 });
    this->getScreen()->blitTextureScale(g_assets.menuScene.keyIns.texture,
        SDL_Rect{ 160,GFX::SCREEN_HEIGHT-30,450 ,0 });

}


OpenScreen::OpenScreen(AppScreen* window, Input* input, SoundManager* audio)
{

    this->window = window;
    this->input = input;
    this->audio = audio;
}
bool OpenScreen::initScene() {
    this->finished = false;
    return true;
}
void OpenScreen::quitScene() {

}

void OpenScreen::handleInput()
{
    if (input->pressed(SDL_SCANCODE_RETURN))
    {
        this->fadeScale = 1;
    }
}
void OpenScreen::update()
{
    if (fadeScale >= 1)
    {
        fadeScale+=4;
    }
    if (fadeScale > 255)
    {
        finished = true;
    }
}
void OpenScreen::render()
{
    this->getScreen()->blitTextureScale(g_assets.menuScene.gameStartImg.texture, SDL_Rect{ 0,0,GFX::SCREEN_WIDTH ,GFX::SCREEN_HEIGHT });
    this->getScreen()->drawRect(SDL_Rect{0,0, GFX::SCREEN_WIDTH, GFX::SCREEN_HEIGHT}, SDL_Color{0,0,0,(unsigned char)(fadeScale)});
}




MenuScene::MenuScene(AppScreen* window, Input* input, SoundManager* audio):data({})
{

    this->window = window;
    this->input = input;
    this->audio = audio;
}
bool MenuScene::initScene() {
    this->finished = false;
    this->deadPlayer = false;
    data.currentLvl = 1;
    this->newGame = false;
    this->firstRun = false;
    data.highestScore = 0;
    //this->menuScene = std::make_unique<OpenScreen>(this->window, this->input, this->audio);
    this->menuScene = std::make_unique<BaseMenu>(this->window, this->input, this->audio);
    return true;
}
void MenuScene::handleInput()
{
}
void MenuScene::update()
{
    this->menuScene.get()->handleInput();
    this->menuScene.get()->update();
    auto* ptrScene = this->menuScene.get();
    if (ptrScene->isFinished())
    {
        auto* baseMenuPtr = dynamic_cast<BaseMenu*>(ptrScene);
        auto* openScreenPtr = dynamic_cast<OpenScreen*>(ptrScene);
        if (baseMenuPtr)// Options Menu
        {
            switch (baseMenuPtr->getOptionIndex())
            {
            //escape/quit
            case -1: {
                if (!this->deadPlayer && !firstRun)//can continue?
                {
                    this->finished = true;
                    newGame = false;
                }
                else// i am not done with you
                {
                    ptrScene->setFinished(false);
                    return;
                }
            }break;
            //new game
            case 0: {
                this->finished = true;
                newGame = true;
            }break;
            //sound
            case 1: {}break;
            //control
            case 2: {}break;
            default:
                break;
            }
        }
        else if(openScreenPtr)
        {
            ptrScene->quitScene();
            this->menuScene = std::make_unique<BaseMenu>(this->window, this->input, this->audio);
        }
    }
    this->menuScene.get()->render();

}
void MenuScene::render() {


}
void MenuScene::quitScene() {


}

