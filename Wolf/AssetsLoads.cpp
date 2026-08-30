#include "pch.hpp"
#include "AssetsLoads.hpp"
#include <SDL_image.h>

bool ImgTexture::load(SDL_Renderer* renderer, const std::string& path)
{
    SDL_Surface* surface = IMG_Load(path.c_str());
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return false;
    return true;
}
void ImgTexture::destroy()
{
    if (texture)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

void Textures::loadAll(SDL_Renderer* renderer) {
    bool success = true;
    success &= wallTMap.load(renderer, "Assets/wallTMap.png");
    success &= guard.load(renderer, "Assets/guard.png");
    success &= weapons.load(renderer, "Assets/weapons.png");
    success &= decorationsTMap.load(renderer, "Assets/Objects.png");
    success &= collectibleTMap.load(renderer, "Assets/collectibles.png");
    success &= statsBar.load(renderer, "Assets/statsBar.png");
    success &= gunsStatsTMap.load(renderer, "Assets/guns_stats_bar.png");
    success &= facesTMap.load(renderer, "Assets/faceswlf.png");

}
void Textures::destroyAll() {
    wallTMap.destroy();
    guard.destroy();
    weapons.destroy();
    decorationsTMap.destroy();
    collectibleTMap.destroy();
    statsBar.destroy();
    gunsStatsTMap.destroy();
    facesTMap.destroy();


}



Textures g_assets;