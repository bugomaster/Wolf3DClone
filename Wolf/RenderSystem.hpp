#pragma once
#include "System.hpp"
#include <sdl_pixels.h>
class GameScene;
class Entity;
class RenderSystem : public System {
public:
    RenderSystem(GameScene* gameScene) :
        gameScene(gameScene)
    {}
    void update(World* world) override;
    static void setFlash(int frames, const SDL_Color& color);
    static int flashTimer;
    static SDL_Color flashColor;
private:
    void drawBackground();
    void renderPlayerStats();
    void renderWeapon();
    void drawWalls();
    void renderDotEntity(Entity* entity);
    void renderDotEntities(World* world);

    GameScene* gameScene;
};