#pragma once
#include "Scene.hpp"
#include "LevelData.hpp"
#include <memory>
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
    LevelData levelData;
public:
    void update() override;
    void handleInput() override{}
    void render() override;
    bool initScene() override;
    void quitScene() override;
    bool isPlayerDead() { return playerDead;}
    void setPlayerDead(bool val) { playerDead = val;}


private:
    bool playerDead = false;
};

struct GameData
{
    int highestScore;
    int currentLvl;
    
};
struct AnimComp
{
    AnimComp(std::vector<int> ids, int frames, int ticksPerFrame) {
        this->ids = ids;
        this->frames = frames;
        this->ticksPerFrame = ticksPerFrame;
    }

    void tick()
    {
        counter++;
        if (counter == ticksPerFrame) {
            frameID++;
            if (frameID == frames)
            {
                frameID = 0;
            }
            counter = 0;
        }
    }
    int getFrameID() {
        return ids.at(frameID);
    }
private:
    int counter = 0;
    int frameID = 0;
    std::vector<int> ids; int frames; int ticksPerFrame;
};
class BaseMenu : public Scene {
public:
    BaseMenu(AppScreen* window, Input* input, SoundManager* audio);

public:
    void update() override;
    void handleInput() override;
    void render() override;
    bool initScene() override;
    void quitScene() override;
    int getOptionIndex() { return cursorIndex; }

private:
    int cursorIndex = 0;
    AnimComp cursorAnim;
    const SDL_Color menuBGColor = { 136, 0,0 ,255 };
    const SDL_Color otherBGColor = { 88, 0,0 ,255 };
};


class MenuScene : public Scene {
public:
    MenuScene(AppScreen* window, Input* input, SoundManager* audio);

public:
    void update() override;
    void handleInput() override;
    void render() override;
    bool initScene() override;
    void quitScene() override;
    void setPlayerDead(bool val) { deadPlayer = val; }
    bool isNewGame() { return this->newGame; }
    bool firstRun = false;
private:

    bool deadPlayer = false;
    bool newGame = false;
    std::unique_ptr<Scene> menuScene;
    GameData data;
};
