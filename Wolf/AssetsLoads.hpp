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

struct Textures
{
public:
    ImgTexture wallTMap;
    ImgTexture guard;
    ImgTexture weapons;
    ImgTexture decorationsTMap;
    ImgTexture collectibleTMap;
    ImgTexture statsBar;
    ImgTexture facesTMap;
    ImgTexture gunsStatsTMap;



    void loadAll(SDL_Renderer* renderer);
    void destroyAll();
};


extern Textures g_assets;