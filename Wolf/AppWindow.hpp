#pragma once

#include <string>
#include <unordered_map>

#include <SDL_stdinc.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

typedef struct _TTF_Font TTF_Font;





class AppScreen {
public:
    SDL_Window* window;
    SDL_Renderer* renderer;       
    bool init();
    void destroy();
    void update();                

    //==============
    void fade(Uint8 alpha);
    void blitTexture(SDL_Texture* tex, int x, int y);
    void blitTextureScale(SDL_Texture* tex, int x, int y, int w, int h);
    void blitTextureScale(SDL_Texture* tex, const SDL_Rect& rect);


    void blitTextureScaleCenter(SDL_Texture* tex, int x, int y, int w, int h);
    void blitPixelsFromTextureScale(SDL_Texture* tex, const SDL_Rect& src, const SDL_Rect& dst, bool flipHor = false, bool flipVer = false);
    void blitPixelsFromTextureScaleRotate(SDL_Texture* tex, const SDL_Rect& src, const SDL_Rect& dst, float angle,bool flipHor = false, bool flipVer = false);
    void drawRect(int x, int y, int w, int h, const SDL_Color& color, bool fill = true, int outLineSize = 1);
    void drawRect(const SDL_Rect& dstRect, const SDL_Color& color, bool fill = true, int outLineSize = 1);
    void drawRect(const SDL_FRect& dstRect, const SDL_Color& color, bool fill = true, int outLineSize = 1);
    void renderText(int x, int y, int size,
        const std::string& text, const SDL_Color& color);


    void blitTextureScaleCenterRotate(SDL_Texture* tex, int x, int y, int w, int h, float angle);
    void clear(SDL_Color color = { 0, 0, 0, 255 });

    TTF_Font* getFont(int size);
    std::unordered_map<int, TTF_Font*> fonts;
};
