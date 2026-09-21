#pragma once
#include <string>

struct SDL_Renderer;
struct SDL_Texture;

struct ImgTexture
{
public:
    SDL_Texture* texture = nullptr;
    bool load(SDL_Renderer* renderer, const std::string& path);
    void destroy();

};    

struct MenuSceneAssets
{
public:
    ImgTexture controlsLogo;
    ImgTexture gameStartImg;
    ImgTexture keyIns;
    ImgTexture loadGame;
    ImgTexture logo;
    ImgTexture lvllDiff;
    ImgTexture optionPointer;
    ImgTexture optionsLogo;
    ImgTexture paused;
};


struct Textures
{
public:
    ImgTexture wallTMap;
    ImgTexture guard;
    ImgTexture hound;
    ImgTexture weapons;
    ImgTexture decorationsTMap;
    ImgTexture collectibleTMap;
    ImgTexture decorations2TMap;
    ImgTexture statsBar;
    ImgTexture keyIconBlue;
    ImgTexture keyIconGold;
    ImgTexture facesTMap;
    ImgTexture gunsStatsTMap;
    ImgTexture bloodTMap;
    MenuSceneAssets menuScene;


    void loadAll(SDL_Renderer* renderer);
    void destroyAll();
};


extern Textures g_assets;