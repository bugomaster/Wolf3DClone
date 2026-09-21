#include "pch.hpp"
#include "Input.hpp"
#include "SoundManager.hpp"
#include "AppWindow.hpp"
#include "Systems.hpp"
#include "AssetsLoads.hpp"
#include "Components.hpp"

 
#include "GameScene.hpp"

#define DEV


GameScene::GameScene(AppScreen* window, Input* input,SoundManager* audio)
{
    this->window = window;
    this->input = input;
    this->audio = audio;
}

bool GameScene::initScene() {
    this->finished = false;
    this->finishedLevel = false;
    loadLevel(this->level);
    return true;

}
bool GameScene::loadLevel(int level) {
    world = new World();
    levelData = {};
    levelData.loadLevelProperties("Levels/Level_" + std::to_string(level) + "/");
    Vector2i wh = Vector2i{ (int)levelData.map[0].size(),(int)levelData.map.size() };
    for (int y = 0; y < wh.y; y++)
    {
        for (int x = 0; x < wh.x; x++)
        {
            this->wallMap[y][x] = levelData.map.at(y).at(x);

        }

    }
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
    //clear wallmap todo
    std::fill(&wallMap[0][0],&wallMap[0][0] + GFX::MAP_H * GFX::MAP_W, 0);
    this->levelData = {  };
    delete world;
    world = nullptr;
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
        fadeScale += 4;
    }
    if (fadeScale > 255)
    {
        finished = true;
    }
}
void OpenScreen::render()
{
    this->getScreen()->blitTextureScale(g_assets.menuScene.gameStartImg.texture, SDL_Rect{ 0,0,GFX::SCREEN_WIDTH ,GFX::SCREEN_HEIGHT });
    this->getScreen()->drawRect(SDL_Rect{ 0,0, GFX::SCREEN_WIDTH, GFX::SCREEN_HEIGHT }, SDL_Color{ 0,0,0,(unsigned char)(fadeScale) });
}



//


BaseMenu::BaseMenu(AppScreen* window, Input* input, SoundManager* audio, int maxLevel,bool canChooseLevel):
    cursorAnim(std::vector<int>{0,1}, 2, 32)
{

    this->canChooseLevel = canChooseLevel;
    this->maxLevel = maxLevel;
    this->window = window;
    this->input = input;
    this->audio = audio;
}
bool BaseMenu::initScene() {
    this->finished = false;
    if (this->canChooseLevel)
    {
        this->chosenLevel = 1;
    }
    else
    {
        this->chosenLevel = maxLevel;
    }
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
    if (this->canChooseLevel)
    {
        if (input->pressed(SDL_SCANCODE_EQUALS))
        {
            chosenLevel++;
            chosenLevel = std::clamp(chosenLevel, 1, this->maxLevel);
        }
        else if (input->pressed(SDL_SCANCODE_MINUS))
        {
            chosenLevel--;
            chosenLevel = std::clamp(chosenLevel, 1, this->maxLevel);

        }
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



    this->getScreen()->renderText(0, 290 + y_padd, 70, "Level:  " + std::to_string(this->chosenLevel), COLORS::WHITE, false);

}


//
MenuScene::MenuScene(AppScreen* window, Input* input, SoundManager* audio, int maxLevel, bool canChooseLevel, bool firstRun)
{
    this->firstRun = firstRun;
    this->canChooseLevel = canChooseLevel;
    this->maxLevel = maxLevel;
    this->window = window;
    this->input = input;
    this->audio = audio;
}
bool MenuScene::initScene() {

#ifdef NOMENU
    this->finished = true;
    this->newGame = true;
#else
#ifdef DEV
    this->menuScene = std::make_unique<BaseMenu>(this->window, this->input, this->audio, this->maxLevel, this->canChooseLevel);
#else
    this->menuScene = std::make_unique<OpenScreen>(this->window, this->input, this->audio);

#endif 
    this->finished = false;
    this->newGame = false;

#endif // NOMENU
    this->newGame = false;
    this->deadPlayer = false;




    this->menuScene.get()->initScene();
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
            switch (baseMenuPtr->cursorIndex)
            {
            //escape -> continue the game
            case -1: {
                if (this->deadPlayer || firstRun)
                {
                    ptrScene->setFinished(false);
                    return;
                }
                else
                {
                    this->finished = true;
                    newGame = false;
                }

            }break;
            //new game
            case 0: {
                this->chosenLevel = 1;//start over the progress
                if (this->canChooseLevel)
                    this->chosenLevel = baseMenuPtr->chosenLevel;
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
            this->menuScene = std::make_unique<BaseMenu>(this->window, this->input, this->audio, this->maxLevel, this->canChooseLevel);
            this->menuScene.get()->initScene();
            
        }
    }
    this->menuScene.get()->render();

}
void MenuScene::render() {


}
void MenuScene::quitScene() {


}

