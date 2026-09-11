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
    success &= decorations2TMap.load(renderer, "Assets/Decorations.png");

    success &= collectibleTMap.load(renderer, "Assets/collectibles.png");
    success &= statsBar.load(renderer, "Assets/statsBar.png");
    success &= keyIconBlue.load(renderer, "Assets/key1icon.png");
    success &= keyIconGold.load(renderer, "Assets/key2icon.png");
    success &= gunsStatsTMap.load(renderer, "Assets/guns_stats_bar.png");
    success &= facesTMap.load(renderer, "Assets/faceswlf.png");
    success &= bloodTMap.load(renderer, "Assets/bloodTMap.png");

    success &= menuScene.controlsLogo.load(renderer, "Assets/MenuScene/ControlsLogo.png");
    success &= menuScene.gameStartImg.load(renderer, "Assets/MenuScene/gameStartImg.png");
    success &= menuScene.keyIns.load(renderer, "Assets/MenuScene/keyIns.png");
    success &= menuScene.loadGame.load(renderer, "Assets/MenuScene/LoadGame.png");
    success &= menuScene.logo.load(renderer, "Assets/MenuScene/Logo.png");
    success &= menuScene.lvllDiff.load(renderer, "Assets/MenuScene/lvllDiff.png");
    success &= menuScene.optionPointer.load(renderer, "Assets/MenuScene/optionPointer.png");
    success &= menuScene.optionsLogo.load(renderer, "Assets/MenuScene/OptionsLogo.png");
    success &= menuScene.paused.load(renderer, "Assets/MenuScene/Paused.png");

}
void Textures::destroyAll() {
    wallTMap.destroy();
    guard.destroy();
    weapons.destroy();
    decorationsTMap.destroy();
    decorations2TMap.destroy();
    collectibleTMap.destroy();
    statsBar.destroy();
    keyIconGold.destroy();
    keyIconBlue.destroy();
    gunsStatsTMap.destroy();
    facesTMap.destroy();
    bloodTMap.destroy();

    //
    menuScene.controlsLogo.destroy();
    menuScene.gameStartImg.destroy();
    menuScene.keyIns.destroy();
    menuScene.loadGame.destroy();
    menuScene.logo.destroy();
    menuScene.lvllDiff.destroy();
    menuScene.optionPointer.destroy();
    menuScene.optionsLogo.destroy();
    menuScene.paused.destroy();

}



Textures g_assets;